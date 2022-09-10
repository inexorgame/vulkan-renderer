#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// Get a user-friendly name of a physical device
/// @param physical_device The physical device
/// @return A user-friendly name of the physical device
[[nodiscard]] std::string get_physical_device_name(VkPhysicalDevice physical_device);

/// Get the type of a physical device
/// @param graphics_card The physical device
/// @return The type of the physical device
[[nodiscard]] VkPhysicalDeviceType get_physical_device_type(VkPhysicalDevice physical_device);

} // namespace inexor::vulkan_renderer::vk_tools
