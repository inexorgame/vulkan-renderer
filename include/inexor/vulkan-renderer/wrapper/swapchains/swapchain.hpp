#pragma once

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

namespace inexor::vulkan_renderer::wrapper::swapchain {

// Using declaration
using synchronization::Semaphore;
using wrapper::Device;

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
    bool m_vsync_enabled{false};

    /// Call vkGetSwapchainImagesKHR
    /// @exception inexor::vulkan_renderer::VulkanException vkGetSwapchainImagesKHR call failed
    /// @return A std::vector of swapchain images (this can be empty!)
    [[nodiscard]] std::vector<VkImage> get_swapchain_images();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param surface The surface
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    Swapchain(const Device &device, VkSurfaceKHR surface, std::uint32_t width, std::uint32_t height,
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

    [[nodiscard]] VkExtent2D extent() const {
        return m_current_extent;
    }

    [[nodiscard]] const VkSemaphore *image_available_semaphore() const {
        return m_img_available->semaphore();
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

} // namespace inexor::vulkan_renderer::wrapper::swapchain
