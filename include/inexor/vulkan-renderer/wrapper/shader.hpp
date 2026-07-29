#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper {

/// RAII wrapper class for VkShaderModules
class Shader {
    const core::Device &m_device;
    std::string m_name;
    std::string m_entry_point;
    VkShaderStageFlagBits m_shader_stage;
    VkShaderModule m_shader_module{VK_NULL_HANDLE};

public:
    /// Construct a shader module from a SPIR-V file.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param shader_stage The shader type.
    /// @param shader_file_name The name of the SPIR-V shader file to load.
    /// @param entry_point The name of the entry point, "main" by default.
    Shader(const core::Device &m_device, VkShaderStageFlagBits shader_stage, const std::string &shader_file_name,
           const std::string &entry_point = "main");

    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;

    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = delete;

    [[nodiscard]] const std::string &entry_point() const {
        return m_entry_point;
    }

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

} // namespace inexor::vulkan_renderer::wrapper
