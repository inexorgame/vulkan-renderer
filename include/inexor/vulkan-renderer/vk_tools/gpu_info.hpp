#pragma once

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief Print available version of Vulkan API.
/// @note Inexor engine does not use a Vulkan metaloader such as Volk.
void print_driver_vulkan_version();

/// @brief Print information about a gpu's device queue families.
/// @param gpu The regarded gpu.
void print_physical_device_queue_families(VkPhysicalDevice gpu);

/// @brief Print all available Vulkan instance layers.
void print_instance_layers();

/// @brief Print all available Vulkan instance extensions.
void print_instance_extensions();

/// @brief Print all available Vulkan device layers for a specified gpu.
/// @param gpu The regarded gpu.
void print_device_layers(VkPhysicalDevice gpu);

/// @brief Print all available Vulkan device extensions.
/// @param gpu The regarded gpu.
void print_device_extensions(VkPhysicalDevice gpu);

/// @brief Print all supported surface capabilities of a given combination of gpu and surface.
/// @param gpu The regarded gpu.
/// @param surface The regarded surface.
void print_surface_capabilities(VkPhysicalDevice gpu, VkSurfaceKHR surface);

/// @brief Print all supported surface formats of a given combination of gpu and surface.
/// @param gpu The regarded gpu.
/// @param surface The regarded Vulkan window surface.
void print_supported_surface_formats(VkPhysicalDevice gpu, VkSurfaceKHR surface);

/// @brief Print all available presentation modes.
/// @param gpu The regarded gpu.
/// @param surface The regarded surface.
void print_presentation_modes(VkPhysicalDevice gpu, VkSurfaceKHR surface);

/// @brief Print information about the specified gpu.
/// @param gpu The regarded gpu.
void print_physical_device_info(VkPhysicalDevice gpu);

/// @brief Print information about the limits of the specified gpu.
/// @param gpu The regarded gpu.
void print_physical_device_limits(VkPhysicalDevice gpu);

/// @brief Print information about the sparse properties of the specified gpu.
/// @param gpu The regarded gpu.
void print_physical_device_sparse_properties(VkPhysicalDevice gpu);

/// @brief Print information about the features of the gpu.
/// @param gpu The regarded gpu.
void print_physical_device_features(VkPhysicalDevice gpu);

/// @brief Print information about the memory properties of a specified gpu.
/// @param gpu The regarded gpu.
void print_physical_device_memory_properties(VkPhysicalDevice gpu);

/// @brief List up all available gpus.
/// @param instance The instance of Vulkan.
/// @param surface The regarded Vulkan window surface.
void print_all_physical_devices(VkInstance instance, VkSurfaceKHR surface);

} // namespace inexor::vulkan_renderer::vk_tools
