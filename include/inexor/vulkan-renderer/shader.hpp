#pragma once

#include <vma/vk_mem_alloc.h>
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
    /// @brief Creates a shader from a SPIR-V memory block.
    Shader(VkDevice device, VkShaderStageFlagBits type, const std::string &name, const std::vector<char> &code, const std::string &entry_point = "main");

    /// @brief Creates a shader from a SPIR-V file.
    Shader(VkDevice device, VkShaderStageFlagBits type, const std::string &name, const std::string &file_name, const std::string &entry_point = "main");

    Shader(const Shader &) = delete;
    Shader(Shader && shader) noexcept;
    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) noexcept = default;

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
