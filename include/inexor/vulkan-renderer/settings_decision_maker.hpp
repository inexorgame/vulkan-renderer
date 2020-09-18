#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer {

/// @brief This class makes automatic decisions about setting up Vulkan.
/// Examples:
/// - Which graphics card will be used if more than one is available?
/// - Which surface color format should be used?
/// - Which graphics card's queue families should be used?
/// - Which presentation modes should be used?
struct VulkanSettingsDecisionMaker {

    /// @brief Rate a graphcs card by its features.
    /// @note The current implementation just sums up all available memory as a score.
    /// @todo Implement additional graphics card rating criteria if desired.
    /// @param graphics_card The graphics card which will be rated.
    /// @return The graphics card's score which is greater or equal to 0.
    [[nodiscard]] std::size_t rate_graphics_card(const VkPhysicalDevice &graphics_card);

    /// @brief Automatically decide if a graphics card is suitable for this application's purposes.
    /// In order to be a suitable graphcs card for Inexor's purposes, it must fulfill the following criteria:
    /// - It must support a swapchain.
    /// - It must support presentation.
    /// @todo Add more checks to the validation mechanism if neccesary, e.h. check for geometry shader support.
    /// @param graphics_card The graphics card which will be checked for suitability.
    /// @return ``true`` if the graphics card is suitable.
    /// @warning When implementing additional graphics card suitability criteria, do not return false for graphics cards
    /// which are not VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU!
    [[nodiscard]] bool is_graphics_card_suitable(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface);

    /// @brief Gets the VkPhysicalDeviceType of a graphics card.
    /// @param graphics_card The graphics card.
    /// @return The type of the graphics card.
    [[nodiscard]] VkPhysicalDeviceType graphics_card_type(const VkPhysicalDevice &graphics_card);

    /// @brief Automatically select the best graphics card considering all available ones.
    /// Please take a look at Inexor's advanced device selection mechanism which is build into this method.
    /// If there is only one graphics card available, we don't have a choice and must try that one.
    /// The user can manually specify which graphics card will be used with command line argument --gpu <index>.
    /// Please note that the graphics cards index starts with 0.
    /// @param vulkan_instance A pointer to the Vulkan instance handle.
    /// @param surface The selected (window) surface.
    /// @param preferred_graphics_card_index The preferred graphics card (by array index).
    /// @return A physical device which was chosen if a suitable one could be found, std::nullopt otherwise.
    [[nodiscard]] std::optional<VkPhysicalDevice>
    decide_which_graphics_card_to_use(const VkInstance &vulkan_instance, const VkSurfaceKHR &surface,
                                      const std::optional<std::uint32_t> &preferred_graphics_card_index = std::nullopt);

    /// @brief Automatically decide how many images will be used in the swap chain.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The number of images that will be used in swap chain.
    [[nodiscard]] std::uint32_t decide_how_many_images_in_swapchain_to_use(const VkPhysicalDevice &graphics_card,
                                                                           const VkSurfaceKHR &surface);

    /// @brief Automatically decide whcih surface color to use in swapchain.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The surface format for swapchain if any could be determined, std::nullopt otherwise.
    [[nodiscard]] std::optional<VkSurfaceFormatKHR>
    decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice &graphics_card,
                                                          const VkSurfaceKHR &surface);

    /// @brief Automatically decide which width and height to use as swapchain extent.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    /// @param swapchain_extent [out] The extent of the swapchain.
    /// @todo Make this function return a std::optional<VkExtend2D> instead of using call by reference!
    void decide_swapchain_extent(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface,
                                 std::uint32_t &window_width, std::uint32_t &window_height,
                                 VkExtent2D &swapchain_extent);

    /// @brief Automatically find the image transform, relative to the presentation engine's natural orientation,
    /// applied to the image content prior to presentation.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The image transform flags.
    [[nodiscard]] VkSurfaceTransformFlagsKHR
    decide_which_image_transformation_to_use(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface);

    /// @brief Find a supported composite alpha format.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The composite alpha flag bits.
    [[nodiscard]] VkCompositeAlphaFlagBitsKHR find_composite_alpha_format(VkPhysicalDevice selected_graphics_card,
                                                                          VkSurfaceKHR surface);

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
    [[nodiscard]] std::optional<VkPresentModeKHR>
    decide_which_presentation_mode_to_use(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface,
                                          bool vsync = false);

    /// @brief Decide which graphics queue family index to use in case it is not possible to use one for both graphics
    /// and presentation.
    /// @warning This function should only be used when it is not possible to use one queue family for both graphics and
    /// presentation!
    /// @param graphics_card The selected graphics card.
    /// @return The index of the queue family which can be used for graphics.
    [[nodiscard]] std::optional<std::uint32_t> find_graphics_queue_family(const VkPhysicalDevice &graphics_card);

    /// @brief Decide which presentation queue family index to use in case it is not possible to use one for both
    /// graphics and presentation.
    /// @warning This function should only be used when it is not possible to use one queue family for both graphics and
    /// presentation!
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The index of the queue family which can be used for presentation.
    [[nodiscard]] std::optional<std::uint32_t> find_presentation_queue_family(const VkPhysicalDevice &graphics_card,
                                                                              const VkSurfaceKHR &surface);

    /// @brief Check if there is a queue family (index) which can be used for both graphics and presentation.
    /// @param graphics_card The selected graphics card.
    /// @param surface The selected (window) surface.
    /// @return The queue family index which can be used for both graphics and presentation (if existent), std::nullopt
    /// otherwise.
    [[nodiscard]] std::optional<std::uint32_t>
    find_queue_family_for_both_graphics_and_presentation(const VkPhysicalDevice &graphics_card,
                                                         const VkSurfaceKHR &surface);

    /// @brief Find a queue family which has VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT.
    /// @warning It might be the case that there is no distinct queue family available on your system!
    /// This means that find_distinct_data_transfer_queue_family must be called to find any queue family which
    /// has VK_QUEUE_TRANSFER_BIT (besides other flags).
    /// @param graphics_card The selected graphics card.
    /// @return The index of the queue family which can be used exclusively  for data transfer.
    [[nodiscard]] std::optional<std::uint32_t>
    find_distinct_data_transfer_queue_family(const VkPhysicalDevice &graphics_card);

    /// @brief Find a queue family which supports VK_QUEUE_TRANSFER_BIT.
    /// @warning You should try to find a distinct queue family first using find_distinct_data_transfer_queue_family!
    /// Distinct queue families have VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT.
    /// It is very likely that the queue family which can be found using this method has VK_QUEUE_GRAPHICS_BIT as well.
    /// This could result in suboptimal use of queues which could decrease performance.
    /// @param graphics_card The selected graphics card.
    /// @return The index of the queue family which can be used for data transfer.
    /// @return A queue family index which can be used for data transfer if any could be found, std::nullopt otherwise.
    [[nodiscard]] std::optional<std::uint32_t>
    find_any_data_transfer_queue_family(const VkPhysicalDevice &graphics_card);

    /// @brief Find a suitable depth buffer format.
    /// @param graphics_card The selected graphics card.
    /// @param formats The depth buffer formats to check for.
    /// @param tiling The desired depth buffer's image tiling.
    /// @param feature_flags The desired depth buffer's feature flags.
    /// @return A VkFormat value if a suitable depth buffer format could be found, std::nullopt otherwise.
    [[nodiscard]] std::optional<VkFormat> find_depth_buffer_format(const VkPhysicalDevice &graphics_card,
                                                                   const std::vector<VkFormat> &formats,
                                                                   VkImageTiling tiling,
                                                                   VkFormatFeatureFlags feature_flags);
};

} // namespace inexor::vulkan_renderer
