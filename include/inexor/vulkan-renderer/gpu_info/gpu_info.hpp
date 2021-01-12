#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::gpu_info {

/// @brief Convert a VkPresentModeKHR value into the corresponding std::string value.
/// @param present_mode The present mode.
/// @return A std::string which contains the presentation mode.
[[nodiscard]] std::string get_present_mode_name(VkPresentModeKHR present_mode);

/// @brief Convert a VkPhysicalDeviceType value into the corresponding std::string value.
/// @param gpu_type The type of the physical device.
/// @return A std::string which contains the physical device type.
[[nodiscard]] std::string get_graphics_card_type(VkPhysicalDeviceType gpu_type);

/// @brief Convert a VkFormat value into the corresponding value as std::string.
/// @param format The VkFormat to convert.
/// @return A std::string which contains the VkFormat.
[[nodiscard]] std::string get_vkformat_name(const VkFormat format);

/// @brief Query which version of the Vulkan API is supported on this system.
/// @note Inexor engine does not use a Vulkan meta loader like volk.
void print_driver_vulkan_version();

/// @brief Print information about a graphics card's device queue families.
/// @param graphics_card The regarded graphics card.
void print_physical_device_queue_families(VkPhysicalDevice graphics_card);

/// @brief Print which instance layers are available for the regarded graphics card on this system.
void print_instance_layers();

/// @brief Print which instance extensions are available for the regarded graphics card on this system.
void print_instance_extensions();

/// @brief Print which device layers are available for the regarded graphics card on this system.
/// @param graphics_card The regarded graphics card.
void print_device_layers(VkPhysicalDevice graphics_card);

/// @brief Print which device extensions are available for the regarded graphics card on this system.
/// @param graphics_card The regarded graphics card.
void print_device_extensions(VkPhysicalDevice graphics_card);

/// @brief Print supported surface capabilities of the regarded combination of graphics card and surface.
/// @param graphics_card The regarded graphics card.
/// @param vulkan_surface The regarded surface.
void print_surface_capabilities(VkPhysicalDevice graphics_card, VkSurfaceKHR vulkan_surface);

/// @brief Print supported surface formats of the regarded combination of graphics card and surface.
/// @param graphics_card The regarded graphics card.
// @param vulkan_surface The regarded Vulkan (window) surface.
void print_supported_surface_formats(VkPhysicalDevice graphics_card, VkSurfaceKHR vulkan_surface);

/// @brief List up all supported presentation modes.
/// @param graphics_card The regarded graphics card.
/// @param vulkan_surface The regarded surface.
void print_presentation_modes(VkPhysicalDevice graphics_card, VkSurfaceKHR vulkan_surface);

/// @brief Print the information about the graphics card.
/// @param graphics_card The regarded graphics card.
void print_physical_device_info(VkPhysicalDevice graphics_card);

/// @brief Print information about the limits of the graphics card.
/// @param graphics_card The regarded graphics card.
void print_physical_device_limits(VkPhysicalDevice graphics_card);

/// @brief Print information about the sparse properties of the graphics card.
/// @param graphics_card The regarded graphics card.
void print_physical_device_sparse_properties(VkPhysicalDevice graphics_card);

/// @brief Print information about the features of the graphics card.
/// @param graphics_card The regarded graphics card.
void print_physical_device_features(VkPhysicalDevice graphics_card);

/// @brief Print information about the graphics card's memory properties.
/// @param graphics_card The regarded graphics card.
void print_physical_device_memory_properties(VkPhysicalDevice graphics_card);

/// @brief List up all available physical devices.
/// @param vulkan_instance The instance of Vulkan.
/// @param vulkan_surface The regarded Vulkan (window) surface.
void print_all_physical_devices(VkInstance vulkan_instance, VkSurfaceKHR vulkan_surface);

} // namespace inexor::vulkan_renderer::gpu_info
