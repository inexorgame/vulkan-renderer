#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <vector>

namespace inexor::vulkan_renderer::wrapper {

struct ShaderLoaderJob {
    std::string file_name;
    VkShaderStageFlagBits shader_type;
    std::string debug_name;
};

class ShaderLoader {
private:
    const std::vector<ShaderLoaderJob> m_shader_files;
    std::vector<Shader> m_shaders;
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

public:
    ShaderLoader(const Device &device, const std::vector<ShaderLoaderJob> &jobs);

    [[nodiscard]] const auto &shaders() const {
        return m_shaders;
    }

    [[nodiscard]] const auto &shader_stages() const {
        return m_shader_stages;
    }

    [[nodiscard]] auto shader_stage_count() const {
        return static_cast<std::uint32_t>(m_shader_stages.size());
    }
};

} // namespace inexor::vulkan_renderer::wrapper
