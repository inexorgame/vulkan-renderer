#pragma once

#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <volk.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::swapchains {

// Using declaration
using synchronization::Semaphore;
using wrapper::commands::CommandBuffer;
using wrapper::core::Device;

/// RAII wrapper class for swapchains
class Swapchain {
private:
    const Device &m_device;
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surface_format;
    std::vector<VkImage> m_imgs;
    std::vector<VkImageView> m_img_views;
    VkExtent2D m_current_extent{};
    std::vector<std::unique_ptr<Semaphore>> m_img_available;
    std::vector<std::unique_ptr<Semaphore>> m_rendering_finished;
    std::vector<VkFence> m_imgs_in_flight;
    std::vector<VkFence> m_frame_slot_submission_fences;
    std::string m_name;
    bool m_vsync_enabled{false};
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    VkImage m_current_swapchain_img{VK_NULL_HANDLE};
    VkImageView m_current_swapchain_img_view{VK_NULL_HANDLE};
    std::uint32_t m_current_frame_slot{0};
    std::uint32_t m_current_img_index{0};

    /// Call vkGetSwapchainImagesKHR
    /// @exception inexor::vulkan_renderer::VulkanException vkGetSwapchainImagesKHR call failed
    /// @return A std::vector of swapchain images (this can be empty!)
    [[nodiscard]] std::vector<VkImage> get_swapchain_images();

    std::uint32_t m_frame_index{0};

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the swapchain
    /// @param surface The surface
    Swapchain(const Device &device, std::string name, VkSurfaceKHR surface);

    ~Swapchain();

    /// Call vkAcquireNextImageKHR
    /// @exception VulkanException vkAcquireNextImageKHR failed
    [[nodiscard]] VkResult acquire_next_image();

    /// Change the image layout with a pipeline barrier to prepare for rendering
    /// @param cmd_buf The command buffer used for recording
    void change_image_layout_to_prepare_for_rendering(const CommandBuffer &cmd_buf);

    /// Change the image layout with a pipeline barrier to prepare to call vkQueuePresentKHR
    /// @param cmd_buf The command buffer used for recording
    void change_image_layout_to_prepare_for_presenting(const CommandBuffer &cmd_buf);

    [[nodiscard]] auto current_swapchain_image_view() const {
        return m_current_swapchain_img_view;
    }

    [[nodiscard]] VkExtent2D extent() const {
        return m_current_extent;
    }

    [[nodiscard]] const VkSemaphore image_available_semaphore() const {
        return m_img_available[m_current_frame_slot % m_img_available.size()]->semaphore();
    }

    [[nodiscard]] VkSemaphore rendering_finished_semaphore() const {
        return m_rendering_finished[m_current_img_index % m_rendering_finished.size()]->semaphore();
    }

    [[nodiscard]] std::uint32_t current_frame_slot() const {
        return m_img_available.empty() ? 0u : m_current_frame_slot;
    }

    [[nodiscard]] std::uint32_t frame_slot_count() const {
        return static_cast<std::uint32_t>(m_img_available.size());
    }

    /// Wait for the fence associated with the currently acquired swapchain image, if any.
    void wait_for_current_image_if_in_flight() const;

    /// Mark the currently acquired swapchain image as owned by the given submission fence.
    void mark_current_image_in_flight(VkFence fence);

    /// Mark the current frame slot as using the given submission fence.
    void mark_current_frame_slot_in_flight(VkFence fence);

    [[nodiscard]] std::uint32_t image_count() const {
        return static_cast<std::uint32_t>(m_imgs.size());
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_surface_format.format;
    }

    [[nodiscard]] const std::vector<VkImageView> &image_views() const {
        return m_img_views;
    }

    [[nodiscard]] const auto &name() const {
        return m_name;
    }

    void present(std::span<const VkSemaphore> wait_semaphores);

    /// Setup the swapchain
    /// @param extent The extent of the swapchain.
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled.
    /// @exception VulkanException vkCreateSwapchainKHR call failed
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    void setup_swapchain(VkExtent2D extent, bool vsync_enabled);

    [[nodiscard]] const VkSwapchainKHR swapchain() const {
        return m_swapchain;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::swapchains
