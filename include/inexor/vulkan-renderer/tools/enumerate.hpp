#pragma once

#include <volk.h>

#include <vector>

namespace inexor::vulkan_renderer::tools {

/// Call vkEnumerateDeviceExtensionProperties
/// @note Because device layers are deprecated in Vulkan, we are not exposing the ``pLayerName`` parameter of
/// ``vkEnumerateDeviceExtensionProperties`` as a parameter here
/// @param physical_device The physical device to get all extension properties form
/// @exception VulkanException vkEnumerateDeviceExtensionProperties call failed
/// @return A std::vector of all device extension properties of a physical device (this can be empty!)
[[nodiscard]] std::vector<VkExtensionProperties> get_extension_properties(VkPhysicalDevice physical_device);

/// Get all available instance extensions.
/// @return The available instance extensions.
[[nodiscard]] std::vector<VkExtensionProperties> get_instance_extensions();

/// Get all available instance layers.
/// @return The available instance layers.
[[nodiscard]] std::vector<VkLayerProperties> get_instance_layers();

/// Call vkEnumeratePhysicalDevices
/// @param inst The Vulkan instance
/// @exception VulkanException vkEnumeratePhysicalDevices call failed
/// @return A std::vector of all physical devices which are available on the system (this can be empty!)
[[nodiscard]] std::vector<VkPhysicalDevice> get_physical_devices(VkInstance inst);

/// Call vkGetPhysicalDeviceQueueFamilyProperties
/// @param physical_device The physical device to get all extension properties form
/// @exception VulkanException vkGetPhysicalDeviceQueueFamilyProperties call failed
/// @return A std::vector of all queue families which are available on the system (this can be empty!)
[[nodiscard]] std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice physical_device);

/// Call vkGetPhysicalDeviceSurfacePresentModesKHR
/// @param physical_device The physical device
/// @param surface The surface
/// @exception VulkanException vkGetPhysicalDeviceSurfaceFormatsKHR call failed
/// @return A std::vector of surface formats
[[nodiscard]] std::vector<VkSurfaceFormatKHR> get_surface_formats(VkPhysicalDevice physical_device,
                                                                  VkSurfaceKHR surface);

/// Call vkGetPhysicalDeviceSurfacePresentModesKHR
/// @param physical_device The physical device
/// @param surface The surface
/// @exception VulkanException vkGetPhysicalDeviceSurfacePresentModesKHR call failed
/// @return A std::vector of present modes
[[nodiscard]] std::vector<VkPresentModeKHR> get_surface_present_modes(VkPhysicalDevice physical_device,
                                                                      VkSurfaceKHR surface);

} // namespace inexor::vulkan_renderer::tools
