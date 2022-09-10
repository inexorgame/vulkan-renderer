#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

namespace inexor::vulkan_renderer::vk_tools {

/// Call vkEnumerateDeviceExtensionProperties
/// @warning If the vkEnumerateDeviceExtensionProperties call fails, a VulkanException is thrown! It is the
/// responsibility of the caller to check and account for this!
/// @warning This can return an empty vector! It is the responsibility of the caller to check and account for this!
/// @note Because device layers are deprecated in Vulkan, we are not exposing ``pLayerName`` parameter of
/// ``vkEnumerateDeviceExtensionProperties`` as a parameter here
/// @param layer_name The name of the (empty by default)
/// @return A std::vector of all device extension properties of a physical device
[[nodiscard]] std::vector<VkExtensionProperties>
get_all_physical_device_extension_properties(VkPhysicalDevice physical_device);

/// Call vkEnumeratePhysicalDevices
/// @warning If the vkEnumeratePhysicalDevices call fails, a VulkanException is thrown! It is the responsibility of the
/// caller to check and account for this!
/// @warning This can return an empty vector! It is the responsibility of the caller to check and account for this!
/// @param inst The Vulkan instance
/// @return A std::vector of all physical devices which are available on the system
[[nodiscard]] std::vector<VkPhysicalDevice> get_all_physical_devices(VkInstance inst);

} // namespace inexor::vulkan_renderer::vk_tools
