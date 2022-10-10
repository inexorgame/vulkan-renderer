#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

namespace inexor::vulkan_renderer::vk_tools {

/// Call vkEnumerateDeviceExtensionProperties
/// @note Because device layers are deprecated in Vulkan, we are not exposing the ``pLayerName`` parameter of
/// ``vkEnumerateDeviceExtensionProperties`` as a parameter here
/// @param physical_device The physical device to get all extension properties form
/// @exception VulkanException vkEnumerateDeviceExtensionProperties call failed
/// @return A std::vector of all device extension properties of a physical device (this can be empty!)
[[nodiscard]] std::vector<VkExtensionProperties> get_extension_properties(VkPhysicalDevice physical_device);

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

} // namespace inexor::vulkan_renderer::vk_tools
