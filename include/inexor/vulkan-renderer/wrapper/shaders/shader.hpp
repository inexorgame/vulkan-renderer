#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::shaders {

/// RAII wrapper class for VkShaderModules
class Shader {
    const core::Device &m_device;
    std::string m_name;
    VkShaderStageFlagBits m_shader_stage;
    VkShaderModule m_shader_module{VK_NULL_HANDLE};

public:
    /// Construct a shader module from a SPIR-V file.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param shader_file_name The name of the SPIR-V shader file to load.
    Shader(const core::Device &m_device, const std::string &shader_file_name);

    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;

    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = delete;

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] VkShaderModule shader_module() const {
        return m_shader_module;
    }

    [[nodiscard]] VkShaderStageFlagBits shader_stage() const {
        return m_shader_stage;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::shaders
