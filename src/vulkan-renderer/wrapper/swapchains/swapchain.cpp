#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain_utils.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::swapchains {

// Using declaration
using tools::InexorException;
using tools::make_info;
using tools::VulkanException;
using wrapper::commands::CommandBufferBuilder;

Swapchain::Swapchain(const core::Device &device, std::string name, const VkSurfaceKHR surface)
    : m_device(device), m_name(std::move(name)), m_surface(surface) {
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
    if (m_name.empty()) {
        throw InexorException("Error: Swapchain name invalid!");
    }
    spdlog::trace("Creating swapchain '{}'", m_name);
}

VkResult Swapchain::acquire_next_image() {
    if (m_img_available.empty()) {
        throw std::runtime_error("Error: Swapchain has no image-available semaphores!");
    }

    const auto slot_count = static_cast<std::uint32_t>(m_img_available.size());
    auto selected_slot = m_frame_index % slot_count;

    if (!m_frame_slot_submission_fences.empty()) {
        bool found_ready_slot = false;
        for (std::uint32_t offset = 0; offset < slot_count; ++offset) {
            const auto slot = (selected_slot + offset) % slot_count;
            auto &slot_fence = m_frame_slot_submission_fences[slot];
            if (slot_fence == VK_NULL_HANDLE) {
                selected_slot = slot;
                found_ready_slot = true;
                break;
            }

            const auto status = vkWaitForFences(m_device.device(), 1, &slot_fence, VK_TRUE, 0);
            if (status == VK_SUCCESS) {
                slot_fence = VK_NULL_HANDLE;
                selected_slot = slot;
                found_ready_slot = true;
                break;
            }
            if (status != VK_TIMEOUT) {
                throw VulkanException("Error: vkWaitForFences failed!", status, m_name);
            }
        }

        if (!found_ready_slot) {
            auto &slot_fence = m_frame_slot_submission_fences[selected_slot];
            if (slot_fence != VK_NULL_HANDLE) {
                if (const auto result = vkWaitForFences(m_device.device(), 1, &slot_fence, VK_TRUE,
                                                        std::numeric_limits<std::uint64_t>::max());
                    result != VK_SUCCESS) {
                    throw VulkanException("Error: vkWaitForFences failed!", result, m_name);
                }
                slot_fence = VK_NULL_HANDLE;
            }
        }
    }

    m_current_frame_slot = selected_slot;

    const auto result =
        vkAcquireNextImageKHR(m_device.device(), m_swapchain, std::numeric_limits<std::uint64_t>::max(),
                              m_img_available[m_current_frame_slot]->semaphore(), VK_NULL_HANDLE, &m_current_img_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        setup_swapchain(m_current_extent, m_vsync_enabled);
        // NOTE: After recreating the swapchain, we can't immediately attempt to acquire the next image index!
        // Instead, we must poll and process window events, and skip frames in rendergraph until acquiring
        // succeeds. If we realize that swapchain has become invalid in the present() call, we will recreate
        // swapchain in the present() function and just continue. Because window events are then polled at the
        // beginning of each frame, we then reach the acquire_next_image function again.
        return result;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw VulkanException("Error: vkAcquireNextImageKHR failed!", result);
    }

    // Store the current swapchain image and current swapchain image view!
    m_current_swapchain_img = m_imgs[m_current_img_index];
    m_current_swapchain_img_view = m_img_views[m_current_img_index];
    return VK_SUCCESS;
}

void Swapchain::wait_for_current_image_if_in_flight() const {
    const auto in_flight_fence = m_imgs_in_flight[m_current_img_index];
    if (in_flight_fence != VK_NULL_HANDLE) {
        if (const auto result = vkWaitForFences(m_device.device(), 1, &in_flight_fence, VK_TRUE,
                                                std::numeric_limits<std::uint64_t>::max());
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkWaitForFences failed!", result, m_name);
        }
    }
}

void Swapchain::mark_current_image_in_flight(const VkFence fence) {
    m_imgs_in_flight[m_current_img_index] = fence;
}

void Swapchain::mark_current_frame_slot_in_flight(const VkFence fence) {
    if (m_img_available.empty()) {
        return;
    }
    if (m_frame_slot_submission_fences.size() != m_img_available.size()) {
        m_frame_slot_submission_fences.assign(m_img_available.size(), VK_NULL_HANDLE);
    }
    m_frame_slot_submission_fences[m_current_frame_slot] = fence;
}

// @TODO Move to inside of rendergraph
void Swapchain::change_image_layout_to_prepare_for_rendering(CommandBufferBuilder &cmd_buf) {
    cmd_buf.change_image_layout(m_current_swapchain_img, m_format, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

// @TODO Move to inside of rendergraph
void Swapchain::change_image_layout_to_prepare_for_presenting(CommandBufferBuilder &cmd_buf) {
    cmd_buf.change_image_layout(m_current_swapchain_img, m_format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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

void Swapchain::present(const std::span<const VkSemaphore> rendering_finished) {
    const auto present_info = make_info<VkPresentInfoKHR>({
        .waitSemaphoreCount = static_cast<std::uint32_t>(rendering_finished.size()),
        .pWaitSemaphores = rendering_finished.data(),
        // @TODO Batch presenting multiple swapchains into one call
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &m_current_img_index,
    });
    if (const auto result = vkQueuePresentKHR(m_device.graphics_queue(), &present_info); result != VK_SUCCESS) {
        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
            // We need to recreate the swapchain
            setup_swapchain(m_current_extent, m_vsync_enabled);
        } else {
            // Exception is thrown if result is not VK_SUCCESS but also not VK_SUBOPTIMAL_KHR
            throw VulkanException("Error: vkQueuePresentKHR failed!", result);
        }
    }
    if (!m_img_available.empty()) {
        m_frame_index = (m_current_frame_slot + 1) % static_cast<std::uint32_t>(m_img_available.size());
    }
}

void Swapchain::setup_swapchain(const VkExtent2D requested_extent, const bool vsync_enabled) {
    const auto caps = m_device.get_surface_capabilities(m_surface);

    // @TODO Which of these values can be determined once at startup because they are unlikely to change at runtime?

    m_surface_format = choose_surface_format(tools::get_surface_formats(m_device.physical_device(), m_surface));
    const auto available_present_modes = tools::get_surface_present_modes(m_device.physical_device(), m_surface);
    const VkSwapchainKHR old_swapchain = m_swapchain;

    auto format_props = tools::make_info<VkFormatProperties2>();
    vkGetPhysicalDeviceFormatProperties2(m_device.physical_device(), m_surface_format.format, &format_props);

    const auto swapchain_ci = make_info<VkSwapchainCreateInfoKHR>({
        .surface = m_surface,
        .minImageCount = choose_image_count(caps),
        .imageFormat = m_surface_format.format,
        .imageColorSpace = m_surface_format.colorSpace,
        .imageExtent = choose_image_extent(requested_extent, caps, m_current_extent),
        .imageArrayLayers = choose_array_layers(caps),
        .imageUsage = choose_image_usage(caps.supportedUsageFlags, format_props.formatProperties.optimalTilingFeatures),
        // NOTE: We use VK_SHARING_MODE_EXCLUSIVE because we consider multi-queue swapchain setups an antipattern.
        // There is likely no real use case for VK_SHARING_MODE_CONCURRENT which could not be achieved otherwise.
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = choose_transform(caps),
        .compositeAlpha = choose_composite_alpha(caps.supportedCompositeAlpha),
        .presentMode = choose_present_mode(available_present_modes, vsync_enabled),
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    });

    m_format = swapchain_ci.imageFormat;

    spdlog::trace("Creating swapchain");

    if (const auto result = vkCreateSwapchainKHR(m_device.device(), &swapchain_ci, nullptr, &m_swapchain);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSwapchainKHR failed!", result);
    }

    m_device.set_debug_name(m_swapchain, m_name);

    // We must destroy the old swapchain and its image views if specified!
    if (old_swapchain != VK_NULL_HANDLE) {
        // Ensure no in-flight work is still touching old swapchain images/views before destruction.
        m_device.wait_idle(m_device.graphics_queue());
        for (auto *const img_view : m_img_views) {
            vkDestroyImageView(m_device.device(), img_view, nullptr);
        }
        m_imgs.clear();
        m_img_views.clear();
        vkDestroySwapchainKHR(m_device.device(), old_swapchain, nullptr);
    }

    // Keep the actual extent selected by Vulkan/capabilities, not the requested window extent.
    m_current_extent = swapchain_ci.imageExtent;

    m_imgs = get_swapchain_images();

    if (m_imgs.empty()) {
        throw std::runtime_error("Error: Swapchain image count is 0!");
    }

    m_rendering_finished.clear();
    m_img_available.clear();
    for (std::size_t img_index = 0; img_index < m_imgs.size(); img_index++) {
        // Create one "rendering finished" semaphore for this swapchain image
        m_rendering_finished.emplace_back(
            std::make_unique<Semaphore>(m_device, "m_rendering_finished[" + std::to_string(img_index) + "]"));
        // Create one "swapchain image available" semaphore for this swapchain image
        m_img_available.emplace_back(
            std::make_unique<Semaphore>(m_device, "m_img_available[" + std::to_string(img_index) + "]"));
        // Name this swapchain image
        m_device.set_debug_name(m_imgs[img_index], m_name + "::m_imgs[" + std::to_string(img_index) + "]");
    }

    // Reset per-image ownership tracking after (re)creating the swapchain.
    m_imgs_in_flight.assign(m_imgs.size(), VK_NULL_HANDLE);
    m_frame_slot_submission_fences.assign(m_imgs.size(), VK_NULL_HANDLE);
    m_current_frame_slot = 0;
    m_frame_index = 0;

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
            throw VulkanException("Error: vkCreateImageView failed!", result,
                                  "swapchain image view " + std::to_string(img_index));
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

} // namespace inexor::vulkan_renderer::wrapper::swapchains
