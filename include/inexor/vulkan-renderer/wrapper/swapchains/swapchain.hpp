#pragma once

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::synchronization {
// Forward declaration
class Semaphore;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::swapchains {

// Using declaration
using synchronization::Semaphore;
using wrapper::Device;
using wrapper::commands::CommandBuffer;

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
    std::unique_ptr<Semaphore> m_img_available;
    std::string m_name;
    bool m_vsync_enabled{false};
    VkImage m_current_swapchain_img{VK_NULL_HANDLE};
    VkImageView m_current_swapchain_img_view{VK_NULL_HANDLE};
    bool m_prepared_for_rendering{false};

    /// Call vkGetSwapchainImagesKHR
    /// @exception inexor::vulkan_renderer::VulkanException vkGetSwapchainImagesKHR call failed
    /// @return A std::vector of swapchain images (this can be empty!)
    [[nodiscard]] std::vector<VkImage> get_swapchain_images();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the swapchain
    /// @param surface The surface
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    Swapchain(const Device &device, std::string name, VkSurfaceKHR surface, std::uint32_t width, std::uint32_t height,
              bool vsync_enabled);

    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&) noexcept;

    ~Swapchain();

    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) = delete;

    /// Call vkAcquireNextImageKHR
    /// @param timeout (``std::numeric_limits<std::uint64_t>::max()`` by default)
    /// @exception VulkanException vkAcquireNextImageKHR call failed
    /// @return The index of the next image
    [[nodiscard]] std::uint32_t
    acquire_next_image_index(std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max());

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
        return m_img_available->semaphore();
    }

    [[nodiscard]] auto *image_available_semaphore_pointer() const {
        return m_img_available->semaphore_pointer();
    }

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

    /// Call vkQueuePresentKHR
    /// @param img_index The image index
    /// @exception VulkanException vkQueuePresentKHR call failed
    void present(std::uint32_t img_index);

    /// Setup the swapchain
    /// @param extent The extent of the swapchain.
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled.
    /// @exception VulkanException vkCreateSwapchainKHR call failed
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    void setup_swapchain(VkExtent2D extent, bool vsync_enabled);

    [[nodiscard]] const VkSwapchainKHR *swapchain() const {
        return &m_swapchain;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::swapchains
