#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Semaphore;

class Swapchain {
private:
    const wrapper::Device &m_device;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_surface_format;
    VkExtent2D m_extent;

    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::uint32_t m_swapchain_image_count;
    const std::string m_name;
    bool m_vsync_enabled;

    /// @brief (Re)creates the swapchain.
    /// @param window_width [in] The requested width of the window.
    /// @param window_height [in] The requested height of the window.
    /// @note We are passing width and height of the window as reference since the API
    /// needs to check if the swapchain can support the requested resolution.
    void setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width, std::uint32_t window_height);

public:
    Swapchain(const Device &device, const VkSurfaceKHR surface, std::uint32_t window_width, std::uint32_t window_height,
              const bool enable_vsync, const std::string &name);
    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&) noexcept;
    ~Swapchain();

    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) = default;

    /// @brief Acquires the next writable image in the swapchain
    /// @param semaphore A semaphore to signal once image acquisition has completed
    /// @returns The image index
    [[nodiscard]] std::uint32_t acquire_next_image(const Semaphore &semaphore);

    /// @brief The swapchain needs to be recreated if it has been invalidated.
    /// @note We must pass width and height as call by reference!
    /// This happens for example when the window gets resized.
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
