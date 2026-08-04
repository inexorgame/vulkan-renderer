#include "inexor/vulkan-renderer/wrapper/shaders/spirv-reflect.hpp"

#include <spirv_reflect.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::shaders {

VkShaderStageFlagBits get_shader_stage(const std::span<const char> spirv) {
    if (spirv.empty()) {
        throw std::runtime_error("SPIR-V data is empty.");
    }

    SpvReflectShaderModule module{};
    const SpvReflectResult result = spvReflectCreateShaderModule(spirv.size(), spirv.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Failed to create SPIRV-Reflect shader module.");
    }

    const auto stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);
    spvReflectDestroyShaderModule(&module);
    return stage;
}

} // namespace inexor::vulkan_renderer::wrapper::shaders
