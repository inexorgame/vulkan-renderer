#pragma once

#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

struct ShaderLoaderJob {
    std::string file_name;
    VkShaderStageFlagBits shader_type;
    std::string debug_name;
};

class ShaderLoader {
private:
    std::vector<Shader> m_shaders;
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stage_ci;

public:
    ///
    /// @param device A const reference to the device wrapper
    /// @param jobs The shader loader jobs
    ShaderLoader(const Device &device, const std::vector<ShaderLoaderJob> &jobs, std::string job_name);

    [[nodiscard]] const auto &shaders() const {
        return m_shaders;
    }

    [[nodiscard]] const auto &shader_stage_create_infos() const {
        return m_shader_stage_ci;
    }

    [[nodiscard]] std::uint32_t shader_stage_count() const {
        return static_cast<std::uint32_t>(m_shader_stage_ci.size());
    }
};

} // namespace inexor::vulkan_renderer::wrapper
