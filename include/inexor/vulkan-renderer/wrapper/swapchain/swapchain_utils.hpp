#pragma once

#include "inexor/vulkan-renderer/tools/representation.hpp"

#include <spdlog/spdlog.h>
#include <volk.h>

#include <set>
#include <span>

namespace inexor::vulkan_renderer::wrapper::swapchain {

// As a general rule of parameter ordering here: available data first, then requested data.
// This makes it easier to have default parameters for the requested data.

/// @note **Design decition**: It was decided to move all code which deals with finding optimal values for the
/// parameters of swapchain creation and recreation to this file called swapchain_utils, because it reduces the mental
/// complexity of the core swapchain wrapper code, and it helps us to write simple tests for the functions in here.

/// Select the number of swapchain array layers.
/// @note The number of swapchain array layers will likely remain `1` unless we start doing using advanced features,
/// @param caps The capabilities of the used surface.
/// @param requested_img_count The requested number of swapchain array layers.
/// @return The selected number of swapchain array layers.
[[nodiscard]] std::uint32_t choose_array_layers(const VkSurfaceCapabilitiesKHR &caps,
                                                std::uint32_t requested_layer_count = 1);

/// Select a composite alpha for the swapchain.
/// @param supported_composite_alpha The supported composite alpha flags.
/// @param request_composite_alpha The requested composite alpha (`VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR` by default).
/// @return The selected composite alpha.
[[nodiscard]] VkCompositeAlphaFlagBitsKHR
choose_composite_alpha(VkCompositeAlphaFlagsKHR available_composite_alpha,
                       VkCompositeAlphaFlagsKHR request_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

/// Select the number of swapchain images.
/// @note **Design decision**: It was decided to expose `frames_in_flight` as parameter because this will play an
/// important role in the automatic double or triple buffering inside of rendergraph later, which affects swapchains.
/// @param caps The capabilities of the used surface.
/// @param frames_in_flight The number of frames in flight.
/// @return The chosen image count.
[[nodiscard]] std::uint32_t choose_image_count(const VkSurfaceCapabilitiesKHR &caps,
                                               std::uint32_t frames_in_flight = 1);

/// Select a swapchain image extent.
/// @param requested_extent The requested image extent.
/// @param caps The capabilities of the used surface.
/// @param current_extent The current image extent.
/// @return The chosen image extent.
[[nodiscard]] VkExtent2D choose_image_extent(const VkExtent2D &requested_extent, const VkSurfaceCapabilitiesKHR &caps,
                                             const VkExtent2D &current_extent);

/// Select suitable swapchain image usage flags.
/// @param supported_flags The image usage flags supported by the surface that is used.
/// @param supported_format_features The format features.
/// @param requested_flags The requested image usage flags (`VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT` by default).
/// @return The chosen swapchain image usage flags.
[[nodiscard]] VkImageUsageFlags
choose_image_usage(VkImageUsageFlags supported_flags, VkFormatFeatureFlags supported_format_features,
                   VkImageUsageFlags requested_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

/// Select a present mode.
/// @param available_present_modes The available present modes.
/// @param vsync_enabled `true` if vertical synchronization is turned on.
/// @return The selected present mode.
[[nodiscard]] VkPresentModeKHR choose_present_mode(std::span<const VkPresentModeKHR> available_present_modes,
                                                   bool vsync_enabled);

/// Select a swapchain surface format.
/// @note **Design decision**: It's always nice to offer the caller the possibility to specify a custom list of
/// prioritized values which will be used to choose a supported value from. If no custom format priority list is
/// specified (it's empty by default), an internal list will be selected as fallback. If the caller specifies a custom
/// list, but none of the given values are supported by the system, an attempt to use the fallabck list will be carried
/// out as well.
/// @param available_formats The surface formats which are available on the system.
/// @param custom_format_prioriy_list A custom list of surface formats to choose from if available (empty by default).
/// @return The selected surface format.
[[nodiscard]] VkSurfaceFormatKHR
choose_surface_format(std::span<const VkSurfaceFormatKHR> available_formats,
                      std::span<const VkSurfaceFormatKHR> custom_format_prioriy_list = {});

/// Select a swapchain pre transform.
/// @note **Design decision**: It makes no sense to turn this into a function which takes a priority list, because in
/// almost all cases, we would like to have `VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR` anyways. Offering a priority list
/// here makes no sense here because it implies that we want to have various rotations of the surface with varying level
/// of acceptance.
/// @param caps The capabilities of the used surface.
/// @param requested_transform The requested transform (`VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR` by default).
/// @return The selected swapchain transform.
[[nodiscard]] VkSurfaceTransformFlagBitsKHR
choose_transform(const VkSurfaceCapabilitiesKHR &caps,
                 VkSurfaceTransformFlagsKHR requested_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

} // namespace inexor::vulkan_renderer::wrapper::swapchain
