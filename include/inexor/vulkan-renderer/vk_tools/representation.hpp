#pragma once

#include <ktx.h>
#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief Convert a Vulkan data structure into a std::string_view.
template <typename VulkanDataType>
[[nodiscard]] std::string_view as_string(VulkanDataType);

/// @brief Convert a return value into a std::string_view.
template <typename ReturnValueType>
[[nodiscard]] std::string_view result_to_description(ReturnValueType result);

} // namespace inexor::vulkan_renderer::vk_tools
