#pragma once

#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class Semaphore;

/// @brief RAII wrapper class for VkSwapchainKHR.
class Swapchain {
    const wrapper::Device &m_device;
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surface_format;
    VkExtent2D m_extent;

    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::uint32_t m_swapchain_image_count{0};
    std::string m_name;
    bool m_vsync_enabled{false};

    /// @brief Set up the swapchain.
    /// @param old_swapchain The old swapchain.
    /// @note Swapchain recreation can be speeded up drastically when passing the old swapchian.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    void setup_swapchain(VkSwapchainKHR old_swapchain, std::uint32_t window_width, std::uint32_t window_height);

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param surface The surface.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    /// @param enable_vsync True if vertical synchronization is requested, false otherwise.
    /// @param name The internal debug marker name of the VkSwapchainKHR.
    Swapchain(const Device &device, VkSurfaceKHR surface, std::uint32_t window_width, std::uint32_t window_height,
              bool enable_vsync, const std::string &name);

    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&) noexcept;

    ~Swapchain();

    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) = delete;

    /// @brief Call vkAcquireNextImageKHR.
    /// @param semaphore A semaphore to signal once image acquisition has completed.
    [[nodiscard]] std::uint32_t acquire_next_image(const Semaphore &semaphore);

    /// @brief Recreate the swapchain.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    void recreate(std::uint32_t window_width, std::uint32_t window_height);

    [[nodiscard]] const VkSwapchainKHR *swapchain_ptr() const {
        return &m_swapchain;
    }

    [[nodiscard]] VkSwapchainKHR swapchain() const {
        return m_swapchain;
    }

    [[nodiscard]] std::uint32_t image_count() const {
        return m_swapchain_image_count;
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_surface_format.format;
    }

    [[nodiscard]] VkExtent2D extent() const {
        return m_extent;
    }

    [[nodiscard]] VkImageView image_view(std::size_t index) const {
        if (index >= m_swapchain_image_views.size()) {
            throw std::out_of_range("Error: swapchain_image_views has " +
                                    std::to_string(m_swapchain_image_views.size()) + " entries. Requested index " +
                                    std::to_string(index) + " is out of bounds!");
        }

        return m_swapchain_image_views.at(index);
    }

    [[nodiscard]] std::vector<VkImageView> image_views() const {
        return m_swapchain_image_views;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
