#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer {

/// @brief A wrapper class for swapchain settings.
struct SwapchainSettings {
    VkExtent2D window_size;
    VkExtent2D swapchain_size;
};

/// @brief This class makes automatic decisions about setting up Vulkan.
/// Examples:
/// - Which graphics card will be used if more than one is available?
/// - Which surface color format should be used?
/// - Which graphics card's queue families should be used?
/// - Which presentation modes should be used?
struct VulkanSettingsDecisionMaker {
    /// @brief Automatically decide how many images will be used in the swap chain.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The number of images that will be used in swap chain.
    [[nodiscard]] static std::uint32_t swapchain_image_count(VkPhysicalDevice graphics_card, VkSurfaceKHR surface);

    /// @brief Automatically decide which surface color to use in swapchain.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The surface format for swapchain if any could be determined, std::nullopt otherwise.
    [[nodiscard]] static std::optional<VkSurfaceFormatKHR>
    swapchain_surface_color_format(VkPhysicalDevice graphics_card, VkSurfaceKHR surface);

    /// @brief Automatically decide which width and height to use as swapchain extent.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    [[nodiscard]] static SwapchainSettings swapchain_extent(VkPhysicalDevice graphics_card, VkSurfaceKHR surface,
                                                            std::uint32_t window_width, std::uint32_t window_height);

    /// @brief Automatically find the image transform, relative to the presentation engine's natural orientation,
    /// applied to the image content prior to presentation.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The image transform flags.
    [[nodiscard]] static VkSurfaceTransformFlagsKHR image_transform(VkPhysicalDevice graphics_card,
                                                                    VkSurfaceKHR surface);

    /// @brief Find a supported composite alpha format.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The composite alpha flag bits.
    [[nodiscard]] static std::optional<VkCompositeAlphaFlagBitsKHR>
    find_composite_alpha_format(VkPhysicalDevice selected_graphics_card, VkSurfaceKHR surface);

    /// @brief Automatically decide which presentation mode the presentation engine will be using.
    /// @note We can only use presentation modes that are available in the current system. The preferred presentation
    /// mode is VK_PRESENT_MODE_MAILBOX_KHR.
    /// @warning Just checking whether swap extension is supported is not enough because presentation support is a queue
    /// family property! A physical device may support swap chains, but that doesn't mean that all its queue families
    /// also support it.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @param vsync True if vertical synchronization is desired, false otherwise.
    /// @return The presentation mode which will be used by the presentation engine.
    [[nodiscard]] static std::optional<VkPresentModeKHR> decide_present_mode(VkPhysicalDevice graphics_card,
                                                                             VkSurfaceKHR surface, bool vsync = false);

    /// @brief Find a suitable depth buffer format.
    /// @param graphics_card The selected graphics card.
    /// @param formats The depth buffer formats to check for.
    /// @param tiling The desired depth buffer's image tiling.
    /// @param feature_flags The desired depth buffer's feature flags.
    /// @return A VkFormat value if a suitable depth buffer format could be found, std::nullopt otherwise.
    [[nodiscard]] static std::optional<VkFormat> find_depth_buffer_format(VkPhysicalDevice graphics_card,
                                                                          const std::vector<VkFormat> &formats,
                                                                          VkImageTiling tiling,
                                                                          VkFormatFeatureFlags feature_flags);
};

} // namespace inexor::vulkan_renderer
