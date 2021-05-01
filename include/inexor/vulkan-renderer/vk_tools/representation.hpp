#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief Convert a Vulkan data structure into a std::string_view.
template <typename VulkanDataType>
[[nodiscard]] std::string_view as_string(VulkanDataType);

/// @brief Turn a VkResult into a std::string_view.
/// @note This function can be used for both VkResult error and success values.
/// @param result The VkResult return value which will be turned into a std::string_view.
[[nodiscard]] std::string_view result_to_string(VkResult result);

/// @brief Return a VkResult's description text as std::string_view.
/// @note This function can be used for both VkResult error and success values.
/// @param result The VkResult return value which will be turned into a std::string_view.
[[nodiscard]] std::string_view result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::vk_tools
