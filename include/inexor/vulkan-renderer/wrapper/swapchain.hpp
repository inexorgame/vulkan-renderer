#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Swapchain {
private:
    // TODO: Move members which don't need to be members!
    VkDevice device;
    VkPhysicalDevice graphics_card;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSwapchainKHR old_swapchain;
    VkSurfaceFormatKHR surface_format;
    VkExtent2D extent;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::uint32_t images_in_swapchain_count;
    bool vsync_enabled;

    /// @brief
    /// @param
    /// @param
    void setup_swapchain(std::uint32_t &window_width, std::uint32_t &window_height);

public:
    /// Delete the copy constructor so swapchains are move-only objects.
    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&other) noexcept;

    /// Delete the copy assignment operator so swapchains are move-only objects.
    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) noexcept = delete;

    /// @brief
    /// @note We must pass width and height as call by reference!
    Swapchain(const VkDevice device, const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface, std::uint32_t &window_width,
              std::uint32_t &window_height, const bool enable_vsync);

    ~Swapchain();

    /// @brief The swapchain needs to be recreated if it has been invalidated.
    /// @note We must pass width and height as call by reference!
    /// This happens for example when the window gets resized.
    void recreate(std::uint32_t &window_width, std::uint32_t &window_height);

    [[nodiscard]] const auto get_swapchain_ptr() const {
        return &swapchain;
    }

    [[nodiscard]] VkSwapchainKHR get_swapchain() const {
        return swapchain;
    }

    [[nodiscard]] std::uint32_t get_image_count() const {
        return images_in_swapchain_count;
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
