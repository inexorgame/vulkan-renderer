#include "inexor/vulkan-renderer/wrapper/swapchain/swapchain.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain/swapchain_utils.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::swapchain {

// Using declaration
using tools::VulkanException;

Swapchain::Swapchain(const Device &device, const VkSurfaceKHR surface, const std::uint32_t width,
                     const std::uint32_t height, const bool vsync_enabled)
    : m_device(device), m_surface(surface), m_vsync_enabled(vsync_enabled) {
    if (vkCreateSwapchainKHR == nullptr) {
        throw InexorException("Error: Function pointer 'vkCreateSwapchainKHR' is not available!");
    }
    if (vkAcquireNextImageKHR == nullptr) {
        throw InexorException("Error: Function pointer 'vkAcquireNextImageKHR' is not available!");
    }
    if (vkGetSwapchainImagesKHR == nullptr) {
        throw InexorException("Error: Function pointer 'vkGetSwapchainImagesKHR' is not available!");
    }
    if (vkQueuePresentKHR == nullptr) {
        throw InexorException("Error: Function pointer 'vkQueuePresentKHR' is not available!");
    }
    if (vkDestroySwapchainKHR == nullptr) {
        throw InexorException("Error: Function pointer 'vkDestroySwapchainKHR' is not available!");
    }
    m_img_available = std::make_unique<Semaphore>(m_device, "m_img_available");
    setup_swapchain({width, height}, vsync_enabled);
}

Swapchain::Swapchain(Swapchain &&other) noexcept : m_device(other.m_device) {
    m_swapchain = std::exchange(other.m_swapchain, VK_NULL_HANDLE);
    m_surface = std::exchange(other.m_surface, VK_NULL_HANDLE);
    m_surface_format = other.m_surface_format;
    m_imgs = std::move(other.m_imgs);
    m_img_views = std::move(other.m_img_views);
    m_current_extent = other.m_current_extent;
    m_img_available = std::exchange(other.m_img_available, nullptr);
    m_vsync_enabled = other.m_vsync_enabled;
}

std::uint32_t Swapchain::acquire_next_image_index(const std::uint64_t timeout) {
    std::uint32_t img_index = 0;
    if (const auto result = vkAcquireNextImageKHR(m_device.device(), m_swapchain, timeout,
                                                  *m_img_available->semaphore(), VK_NULL_HANDLE, &img_index);
        result != VK_SUCCESS) {
        if (result == VK_SUBOPTIMAL_KHR) {
            // We need to recreate the swapchain.
            setup_swapchain(m_current_extent, m_vsync_enabled);
        } else {
            throw VulkanException("Error: vkAcquireNextImageKHR failed!", result);
        }
    }
    return img_index;
}

std::vector<VkImage> Swapchain::get_swapchain_images() {
    std::uint32_t img_count = 0;
    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &img_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetSwapchainImagesKHR failed!", result);
    }
    std::vector<VkImage> imgs(img_count);
    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &img_count, imgs.data());
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
            setup_swapchain(m_current_extent, m_vsync_enabled);
        } else {
            // Exception is thrown if result is not VK_SUCCESS but also not VK_SUBOPTIMAL_KHR
            throw VulkanException("Error: vkQueuePresentKHR failed!", result);
        }
    }
}

void Swapchain::setup_swapchain(const VkExtent2D requested_extent, const bool vsync_enabled) {
    const auto caps = m_device.get_surface_capabilities(m_surface);

    // @TODO Which of these values can be determined once at startup because they are unlikely to change at runtime?

    m_surface_format = choose_surface_format(tools::get_surface_formats(m_device.physical_device(), m_surface));
    const auto available_present_modes = tools::get_surface_present_modes(m_device.physical_device(), m_surface);
    const VkSwapchainKHR old_swapchain = m_swapchain;

    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(m_device.physical_device(), m_surface_format.format, &format_props);

    const auto swapchain_ci = make_info<VkSwapchainCreateInfoKHR>({
        .surface = m_surface,
        .minImageCount = choose_image_count(caps),
        .imageFormat = m_surface_format.format,
        .imageColorSpace = m_surface_format.colorSpace,
        .imageExtent = choose_image_extent(requested_extent, caps, m_current_extent),
        .imageArrayLayers = choose_array_layers(caps),
        .imageUsage = choose_image_usage(caps.supportedUsageFlags, format_props.optimalTilingFeatures),
        // We use VK_SHARING_MODE_EXCLUSIVE because we consider multi-queue swapchain setups an antipattern.
        // There is likely no real use case for VK_SHARING_MODE_EXCLUSIVE which could not be achieved otherwise.
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = choose_transform(caps),
        .compositeAlpha = choose_composite_alpha(caps.supportedCompositeAlpha),
        .presentMode = choose_present_mode(available_present_modes, vsync_enabled),
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    });

    spdlog::trace("Creating swapchain");

    if (const auto result = vkCreateSwapchainKHR(m_device.device(), &swapchain_ci, nullptr, &m_swapchain);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSwapchainKHR failed!", result);
    }

    // We must destroy the old swapchain and its image views if specified!
    if (old_swapchain != VK_NULL_HANDLE) {
        for (auto *const img_view : m_img_views) {
            vkDestroyImageView(m_device.device(), img_view, nullptr);
        }
        m_imgs.clear();
        m_img_views.clear();
        vkDestroySwapchainKHR(m_device.device(), old_swapchain, nullptr);
    }

    m_current_extent = requested_extent;

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
            .format = m_surface_format.format,
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

} // namespace inexor::vulkan_renderer::wrapper::swapchain
