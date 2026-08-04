#pragma once

#include <volk.h>

#include <span>

namespace inexor::vulkan_renderer::wrapper::shaders {

/// Gets the shader stage from SPIR-V data
/// @param spirv The SPIR-V data as a span of characters
/// @return The corresponding VkShaderStageFlagBits for the shader stage
[[nodiscard]] VkShaderStageFlagBits get_shader_stage(std::span<const char> spirv);

} // namespace inexor::vulkan_renderer::wrapper::shaders
