#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class Shader
/// @brief RAII wrapper class for VkShaderModules.
class Shader {
    const Device &m_device;
    const std::string m_name;
    const std::string m_entry_point;
    VkShaderStageFlagBits m_type;
    VkShaderModule m_shader_module;

public:
    /// @brief Constructs a shader module from a block of SPIR-V memory.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param type [in] The shader type.
    /// @param name
    /// @param code [in] The memory block of the SPIR-V shader.
    /// @param entry_point [in] The name of the entry point, "main" by default.
    Shader(const Device &device, VkShaderStageFlagBits type, const std::string &name, const std::vector<char> &code,
           const std::string &entry_point = "main");

    /// @brief Constructs a shader module from a SPIR-V file.
    /// This constructor loads the file content and just calls the other constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param type [in] The shader type.
    /// @param name [in] The internal debug marker name of the VkShaderModule.
    /// @param file_name [in] The name of the SPIR-V shader file to load.
    /// @param entry_point [in] The name of the entry point, "main" by default.
    Shader(const Device &device, VkShaderStageFlagBits type, const std::string &name, const std::string &file_name,
           const std::string &entry_point = "main");

    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;

    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = default;

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] const std::string &entry_point() const {
        return m_entry_point;
    }

    [[nodiscard]] VkShaderStageFlagBits type() const {
        return m_type;
    }

    [[nodiscard]] VkShaderModule module() const {
        return m_shader_module;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
