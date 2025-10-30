#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

using tools::VulkanException;

Swapchain::Swapchain(Device &device, const VkSurfaceKHR surface, const std::uint32_t width, const std::uint32_t height,
                     const bool vsync_enabled)
    : m_device(device), m_surface(surface), m_vsync_enabled(vsync_enabled) {
    if (vkCreateSwapchainKHR == nullptr) {
        throw std::runtime_error("Error: vkCreateSwapchainKHR is not available! Did you forget to enable "
                                 "VK_KHR_swapchain device extension?");
    }
    m_img_available = std::make_unique<Semaphore>(m_device, "Swapchain image available");
    setup_swapchain(width, height, vsync_enabled);
}

Swapchain::Swapchain(Swapchain &&other) noexcept : m_device(other.m_device) {
    m_swapchain = std::exchange(other.m_swapchain, VK_NULL_HANDLE);
    m_surface = std::exchange(other.m_surface, VK_NULL_HANDLE);
    m_surface_format = other.m_surface_format;
    m_imgs = std::move(other.m_imgs);
    m_img_views = std::move(other.m_img_views);
    m_extent = other.m_extent;
    m_img_available = std::exchange(other.m_img_available, nullptr);
    m_vsync_enabled = other.m_vsync_enabled;
}

std::uint32_t Swapchain::acquire_next_image_index(const std::uint64_t timeout) {
    std::uint32_t img_index = 0;
    if (const auto result = vkAcquireNextImageKHR(m_device.device(), m_swapchain, timeout,
                                                  *m_img_available->semaphore(), VK_NULL_HANDLE, &img_index);
        result != VK_SUCCESS) {
        if (result == VK_SUBOPTIMAL_KHR) {
            // We need to recreate the swapchain
            setup_swapchain(m_extent.width, m_extent.height, m_vsync_enabled);
        } else {
            throw VulkanException("Error: vkAcquireNextImageKHR failed!", result);
        }
    }
    return img_index;
}

std::optional<VkCompositeAlphaFlagBitsKHR>
Swapchain::choose_composite_alpha(const VkCompositeAlphaFlagBitsKHR request_composite_alpha,
                                  const VkCompositeAlphaFlagsKHR supported_composite_alpha) {
    if ((request_composite_alpha & supported_composite_alpha) != 0u) {
        return request_composite_alpha;
    }

    static const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};

    for (const auto flag : composite_alpha_flags) {
        if ((flag & supported_composite_alpha) != 0u) {
            spdlog::trace("Swapchain composite alpha '{}' is not supported, selecting '{}'",
                          tools::as_string(request_composite_alpha), tools::as_string(flag));
            return flag;
        }
    }
    return std::nullopt;
}

VkExtent2D Swapchain::choose_image_extent(const VkExtent2D &requested_extent, const VkExtent2D &min_extent,
                                          const VkExtent2D &max_extent, const VkExtent2D &current_extent) {
    if (current_extent.width == std::numeric_limits<std::uint32_t>::max()) {
        return requested_extent;
    }
    if (requested_extent.width < 1 || requested_extent.height < 1) {
        spdlog::trace("Swapchain image extent ({}, {}) is not supported! Selecting ({}, {})", requested_extent.width,
                      requested_extent.height, current_extent.width, current_extent.height);
        return current_extent;
    }
    return {
        .width = std::clamp(requested_extent.width, min_extent.width, max_extent.width),
        .height = std::clamp(requested_extent.height, min_extent.height, max_extent.height),
    };
}

VkPresentModeKHR Swapchain::choose_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes,
                                                const std::vector<VkPresentModeKHR> &present_mode_priority_list,
                                                const bool vsync_enabled) {
    assert(!available_present_modes.empty());
    assert(!present_mode_priority_list.empty());
    if (!vsync_enabled) {
        for (const auto requested_present_mode : present_mode_priority_list) {
            const auto present_mode =
                std::find(available_present_modes.begin(), available_present_modes.end(), requested_present_mode);
            if (present_mode != available_present_modes.end()) {
                return *present_mode;
            }
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

std::optional<VkSurfaceFormatKHR>
Swapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats,
                                 const std::vector<VkSurfaceFormatKHR> &format_prioriy_list) {
    assert(!available_formats.empty());

    // Try to find one of the formats in the priority list
    for (const auto requested_format : format_prioriy_list) {
        const auto format =
            std::find_if(available_formats.begin(), available_formats.end(), [&](const VkSurfaceFormatKHR candidate) {
                return requested_format.format == candidate.format &&
                       requested_format.colorSpace == candidate.colorSpace;
            });
        if (format != available_formats.end()) {
            spdlog::trace("Selecting swapchain surface format {}", tools::as_string(*format));
            return *format;
        }
    }

    spdlog::trace("None of the surface formats of the priority list are supported");
    spdlog::trace("Selecting surface format from default list");

    static const std::vector<VkSurfaceFormatKHR> default_surface_format_priority_list{
        {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

    std::optional<VkSurfaceFormatKHR> chosen_format{};

    // Try to find one of the formats in the default list
    for (const auto available_format : available_formats) {
        const auto format =
            std::find_if(default_surface_format_priority_list.begin(), default_surface_format_priority_list.end(),
                         [&](const VkSurfaceFormatKHR candidate) {
                             return available_format.format == candidate.format &&
                                    available_format.colorSpace == candidate.colorSpace;
                         });

        if (format != default_surface_format_priority_list.end()) {
            spdlog::trace("Selecting swapchain image format {}", tools::as_string(*format));
            chosen_format = *format;
        }
    }
    // This can be std::nullopt
    return chosen_format;
}

std::vector<VkImage> Swapchain::get_swapchain_images() {
    std::uint32_t count = 0;
    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetSwapchainImagesKHR failed!", result);
    }
    std::vector<VkImage> imgs(count);
    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &count, imgs.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetSwapchainImagesKHR failed!", result);
    }
    return imgs;
}

void Swapchain::present(const std::uint32_t img_index) {
    const auto present_info = make_info<VkPresentInfoKHR>({
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &img_index,
    });
    if (const auto result = vkQueuePresentKHR(m_device.present_queue(), &present_info); result != VK_SUCCESS) {
        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
            // We need to recreate the swapchain
            setup_swapchain(m_extent.width, m_extent.height, m_vsync_enabled);
        } else {
            // Exception is thrown if result is not VK_SUCCESS but also not VK_SUBOPTIMAL_KHR
            throw VulkanException("Error: vkQueuePresentKHR failed!", result);
        }
    }
}

void Swapchain::setup_swapchain(const std::uint32_t width, const std::uint32_t height, const bool vsync_enabled) {
    const auto caps = m_device.get_surface_capabilities(m_surface);
    m_surface_format = choose_surface_format(tools::get_surface_formats(m_device.physical_device(), m_surface));
    const VkExtent2D requested_extent{.width = width, .height = height};

    static const std::vector<VkPresentModeKHR> default_present_mode_priorities{
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
    };

    const auto composite_alpha =
        choose_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, caps.supportedCompositeAlpha);

    if (!composite_alpha) {
        throw std::runtime_error("Error: Could not find suitable composite alpha!");
    }

    if ((caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0u) {
        throw std::runtime_error(
            "Error: Swapchain image usage flag bit VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is not supported!");
    }

    const VkSwapchainKHR old_swapchain = m_swapchain;

    const auto swapchain_ci = make_info<VkSwapchainCreateInfoKHR>({
        .surface = m_surface,
        .minImageCount = (caps.maxImageCount != 0) ? std::min(caps.minImageCount + 1, caps.maxImageCount)
                                                   : std::max(caps.minImageCount + 1, caps.minImageCount),
        .imageFormat = m_surface_format.value().format,
        .imageColorSpace = m_surface_format.value().colorSpace,
        .imageExtent = choose_image_extent(requested_extent, caps.minImageExtent, caps.maxImageExtent, m_extent),
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = ((VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & caps.supportedTransforms) != 0u)
                            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
                            : caps.currentTransform,
        .compositeAlpha = composite_alpha.value(),
        .presentMode = choose_present_mode(tools::get_surface_present_modes(m_device.physical_device(), m_surface),
                                           default_present_mode_priorities, vsync_enabled),
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    });

    spdlog::trace("Using swapchain surface transform {}", tools::as_string(swapchain_ci.preTransform));

    spdlog::trace("Creating swapchain");

    if (const auto result = vkCreateSwapchainKHR(m_device.device(), &swapchain_ci, nullptr, &m_swapchain);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSwapchainKHR failed!", result);
    }

    // We need to destroy the old swapchain if specified
    if (old_swapchain != VK_NULL_HANDLE) {
        for (auto *const img_view : m_img_views) {
            vkDestroyImageView(m_device.device(), img_view, nullptr);
        }
        m_imgs.clear();
        m_img_views.clear();
        vkDestroySwapchainKHR(m_device.device(), old_swapchain, nullptr);
    }

    m_extent.width = width;
    m_extent.height = height;

    m_imgs = get_swapchain_images();

    if (m_imgs.empty()) {
        throw std::runtime_error("Error: Swapchain image count is 0!");
    }

    spdlog::trace("Creating {} swapchain image views", m_imgs.size());

    m_img_views.resize(m_imgs.size());

    for (std::size_t img_index = 0; img_index < m_imgs.size(); img_index++) {
        const auto img_view_ci = make_info<VkImageViewCreateInfo>({
            .image = m_imgs[img_index],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_surface_format.value().format,
            .components{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        });

        if (const auto result = vkCreateImageView(m_device.device(), &img_view_ci, nullptr, &m_img_views[img_index]);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkCreateImageView failed!", result, "swapchain image view " + img_index);
        }
        m_device.set_debug_name(m_img_views[img_index], "swapchain image view");
    }
}

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
    for (auto *const img_view : m_img_views) {
        vkDestroyImageView(m_device.device(), img_view, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
