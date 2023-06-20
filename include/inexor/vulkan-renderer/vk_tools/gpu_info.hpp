#pragma once

#include <volk.h>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief Print available version of Vulkan API
/// @note Inexor engine does not use a Vulkan metaloader such as Volk
void print_driver_vulkan_version();

/// @brief Print information about a physical device's device queue families
/// @param physical_device The regarded physical device
void print_physical_device_queue_families(VkPhysicalDevice physical_device);

/// @brief Print all available Vulkan instance layers
void print_instance_layers();

/// @brief Print all available Vulkan instance extensions
void print_instance_extensions();

// Note that device layers are deprecated

/// @brief Print all available Vulkan device extensions
/// @param physical_device The regarded physical device
void print_device_extensions(VkPhysicalDevice physical_device);

/// @brief Print all supported surface capabilities of a given combination of physical device and surface
/// @param physical_device The regarded physical device
/// @param surface The regarded surface
void print_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/// @brief Print all supported surface formats of a given combination of physical device and surface
/// @param physical_device The regarded physical device
/// @param surface The regarded Vulkan window surface
void print_supported_surface_formats(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/// @brief Print all available presentation modes
/// @param physical_device The regarded physical device
/// @param surface The regarded surface
void print_presentation_modes(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/// @brief Print information about the specified physical device
/// @param physical_device The regarded physical device
void print_physical_device_info(VkPhysicalDevice physical_device);

/// @brief Print information about the limits of the specified physical device
/// @param physical_device The regarded physical device
void print_physical_device_limits(VkPhysicalDevice physical_device);

/// @brief Print information about the sparse properties of the specified physical device
/// @param physical_device The regarded physical device
void print_physical_device_sparse_properties(VkPhysicalDevice physical_device);

/// @brief Print information about the features of the physical device
/// @param physical_device The regarded physical device
void print_physical_device_features(VkPhysicalDevice physical_device);

/// @brief Print information about the memory properties of a specified physical device
/// @param physical_device The regarded physical device
void print_physical_device_memory_properties(VkPhysicalDevice physical_device);

/// @brief List up all available physical devices
/// @param instance The instance of Vulkan
/// @param surface The regarded Vulkan window surface
void print_all_physical_devices(VkInstance instance, VkSurfaceKHR surface);

} // namespace inexor::vulkan_renderer::vk_tools
