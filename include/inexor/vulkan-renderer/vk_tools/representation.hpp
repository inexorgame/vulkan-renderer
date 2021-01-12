#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief Convert a VkPresentModeKHR value into the corresponding std::string value.
/// @param present_mode The present mode.
/// @return A std::string which contains the presentation mode.
[[nodiscard]] std::string present_mode_khr_to_string(VkPresentModeKHR present_mode);

/// @brief Convert a VkPhysicalDeviceType value into the corresponding std::string value.
/// @param gpu_type The type of the physical device.
/// @return A std::string which contains the physical device type.
[[nodiscard]] std::string physical_device_type_to_string(VkPhysicalDeviceType gpu_type);

/// @brief Convert a VkFormat value into the corresponding value as std::string.
/// @param format The VkFormat to convert.
/// @return A std::string which contains the VkFormat.
[[nodiscard]] std::string format_to_string(VkFormat format);

/// @brief Turn a VkResult into a string.
/// @note This function can be used for both VkResult error and success values.
/// @param result The VkResult return value which will be turned into a string.
[[nodiscard]] std::string result_to_string(VkResult result);

///@brief Return a VkResult's description text.
/// @note This function can be used for both VkResult error and success values.
/// @param result The VkResult return value which will be turned into a string.
[[nodiscard]] std::string result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::vk_tools
