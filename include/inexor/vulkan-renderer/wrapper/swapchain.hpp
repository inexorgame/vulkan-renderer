#pragma once

#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Swapchain {
private:
    // TODO: Move members which don't need to be members!
    VkDevice device;
    VkPhysicalDevice graphics_card;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR surface_format;
    VkExtent2D extent;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::uint32_t swapchain_image_count;
    bool vsync_enabled;

    /// @brief (Re)creates the swapchain.
    /// @param window_width [in] The requested width of the window.
    /// @param window_height [in] The requested height of the window.
    /// @note We are passing width and height of the window as reference since the API
    /// needs to check if the swapchain can support the requested resolution.
    void setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width, std::uint32_t window_height);

public:
    /// Delete the copy constructor so swapchains are move-only objects.
    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&other) noexcept;

    /// Delete the copy assignment operator so swapchains are move-only objects.
    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) noexcept = delete;

    /// @brief
    /// @note We must pass width and height as call by reference!
    Swapchain(const VkDevice device, const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface, std::uint32_t window_width, std::uint32_t window_height,
              const bool enable_vsync);

    ~Swapchain();

    /// @brief The swapchain needs to be recreated if it has been invalidated.
    /// @note We must pass width and height as call by reference!
    /// This happens for example when the window gets resized.
    void recreate(std::uint32_t window_width, std::uint32_t window_height);

    [[nodiscard]] const VkSwapchainKHR *get_swapchain_ptr() const {
        return &swapchain;
    }

    [[nodiscard]] VkSwapchainKHR get_swapchain() const {
        return swapchain;
    }

    [[nodiscard]] std::uint32_t get_image_count() const {
        return swapchain_image_count;
    }

    [[nodiscard]] VkFormat get_image_format() const {
        return surface_format.format;
    }

    [[nodiscard]] VkExtent2D get_extent() const {
        return extent;
    }

    [[nodiscard]] VkImageView get_image_view(std::size_t index) const {
        if (index >= swapchain_image_views.size()) {
            throw std::out_of_range("Error: swapchain_image_views has " + std::to_string(swapchain_image_views.size()) + " entries. Requested index " +
                                    std::to_string(index) + " is out of bounds!");
        }

        return swapchain_image_views.at(index);
    }
};

} // namespace inexor::vulkan_renderer::wrapper
