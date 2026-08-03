#pragma once

#include <volk.h>

#include <filesystem>
#include <span>
#include <string_view>

namespace inexor::vulkan_renderer::wrapper::shaders {

[[nodiscard]] VkShaderStageFlagBits get_shader_stage(const std::filesystem::path &path);

[[nodiscard]] VkShaderStageFlagBits get_shader_stage(std::span<const char> spirv);

} // namespace inexor::vulkan_renderer::wrapper::shaders
