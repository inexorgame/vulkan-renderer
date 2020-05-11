#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

class Shader {
private:
    VkDevice device;
    VkShaderStageFlagBits type;
    std::string name;
    std::string entry_point;

    VkShaderModule shader_module;

public:
    /// Delete the copy constructor so shaders are move-only objects.
    Shader(const Shader &) = delete;
    Shader(Shader &&shader) noexcept;

    /// Delete the copy assignment operator so shaders are move-only objects.
    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) noexcept = default;

    /// @brief Creates a shader from memory.
    /// @param device [in] The Vulkan device which will be used to create the shader module.
    /// @param type [in] The shader type (vertex shader, fragment shader, tesselation shader..).
    /// @param name [in] The internal name of the shader module.
    /// @param code [in] The SPIR-V shader code.
    /// @param entry_point [in] The entry point of the shader code, in most cases just "main".
    Shader(VkDevice device, VkShaderStageFlagBits type, const std::string &name, const std::vector<char> &code, const std::string &entry_point = "main");

    /// @brief Creates a shader from a SPIR-V file.
    /// @param device [in] The Vulkan device which will be used to create the shader module.
    /// @param type [in] The shader type (vertex shader, fragment shader, tesselation shader..).
    /// @param name [in] The internal name of the shader module.
    /// @param file_name [in] The name of the SPIR-V shader file.
    /// @param entry_point [in] The entry point of the shader code, in most cases just "main".
    Shader(VkDevice device, VkShaderStageFlagBits type, const std::string &name, const std::string &file_name, const std::string &entry_point = "main");

    ~Shader();

    const std::string &get_name() const {
        return name;
    }

    const std::string &get_entry_point() const {
        return entry_point;
    }

    VkShaderStageFlagBits get_type() const {
        return type;
    }

    VkShaderModule get_module() const {
        return shader_module;
    }
};

} // namespace inexor::vulkan_renderer
