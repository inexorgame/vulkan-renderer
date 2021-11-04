#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkShaderModules.
class Shader {
    const Device &m_device;
    std::string m_name;
    std::string m_entry_point = "main";
    VkShaderStageFlagBits m_type;
    VkShaderModule m_shader_module{VK_NULL_HANDLE};

public:
    /// @brief Construct a shader module from a block of SPIR-V memory.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param type The shader type.
    /// @param name The internal debug marker name of the VkShaderModule.
    /// @param code The memory block of the SPIR-V shader.
    Shader(const Device &m_device, VkShaderStageFlagBits type, const std::vector<char> &code, const std::string &name);

    /// @brief Construct a shader module from a SPIR-V file.
    /// This constructor loads the file content and just calls the other constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param type The shader type.
    /// @param name The internal debug marker name of the VkShaderModule.
    /// @param file_name The name of the SPIR-V shader file to load.
    Shader(const Device &m_device, VkShaderStageFlagBits type, const std::string &file_name, const std::string &name);

    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;

    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = delete;

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
