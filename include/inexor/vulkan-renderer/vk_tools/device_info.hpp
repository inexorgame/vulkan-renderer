#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::vk_tools {

/// Transform a ``VkPhysicalDeviceFeatures`` into a ``std::vector<VkBool32>``
/// @note The size of the vector will be determined by the number of ``VkBool32`` variables in the
/// ``VkPhysicalDeviceFeatures`` struct
/// @param features The physical device features
/// @return A ``std::vector<VkBool32>`` The transformed data
[[nodiscard]] std::vector<VkBool32> get_device_features_as_vector(const VkPhysicalDeviceFeatures &features);

/// Get the name of a physical device
/// @param physical_device The physical device
/// @return The name of the physical device
[[nodiscard]] std::string get_physical_device_name(VkPhysicalDevice physical_device);

} // namespace inexor::vulkan_renderer::vk_tools
