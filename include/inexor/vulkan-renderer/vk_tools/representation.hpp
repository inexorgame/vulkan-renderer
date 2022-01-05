#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief This function returns a textual representation of the vulkan object T.
template <typename T>
[[nodiscard]] std::string_view as_string(T);

/// @brief Convert a VkResult value into the corresponding error description as std::string_view
/// @param result The VkResult to convert
/// @return A std::string_view which contains an error description text of the VkResult
/// @note This function converts the VkResult into the corresponding error description text
/// If you want to convert it into an std::string_view, see the matching ```as_string``` template
[[nodiscard]] std::string_view result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::vk_tools
