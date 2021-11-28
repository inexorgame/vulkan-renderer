#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

template <typename T>
[[nodiscard]] std::string_view as_string(T);

[[nodiscard]] std::string_view result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::vk_tools
