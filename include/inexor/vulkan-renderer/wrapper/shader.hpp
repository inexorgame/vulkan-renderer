#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Shader {
private:
    const Device &m_device;
    VkShaderStageFlagBits m_type;
    const std::string m_name;
    const std::string m_entry_point;

    VkShaderModule m_shader_module;

public:
    /// @brief Creates a shader from memory.
    /// @param device [in] The Vulkan device which will be used to create the shader module.
    /// @param type [in] The shader type (vertex shader, fragment shader, tesselation shader..).
    /// @param name [in] The internal name of the shader module.
    /// @param code [in] The SPIR-V shader code.
    /// @param entry_point [in] The entry point of the shader code, in most cases just "main".
    Shader(const Device &m_device, VkShaderStageFlagBits type, const std::string &name, const std::vector<char> &code,
           const std::string &entry_point = "main");

    /// @brief Creates a shader from a SPIR-V file.
    /// @param device [in] The Vulkan device which will be used to create the shader module.
    /// @param type [in] The shader type (vertex shader, fragment shader, tesselation shader..).
    /// @param name [in] The internal name of the shader module.
    /// @param file_name [in] The name of the SPIR-V shader file.
    /// @param entry_point [in] The entry point of the shader code, in most cases just "main".
    Shader(const Device &m_device, VkShaderStageFlagBits type, const std::string &name, const std::string &file_name,
           const std::string &entry_point = "main");
    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;
    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = default;

    [[nodiscard]] const std::string &name() const {
        assert(!m_name.empty());
        return m_name;
    }

    [[nodiscard]] const std::string &entry_point() const {
        assert(!m_entry_point.empty());
        return m_entry_point;
    }

    [[nodiscard]] VkShaderStageFlagBits type() const {
        return m_type;
    }

    [[nodiscard]] VkShaderModule module() const {
        assert(m_shader_module);
        return m_shader_module;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
