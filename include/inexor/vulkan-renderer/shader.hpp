#pragma once

#include "spdlog/spdlog.h"
#include "vma/vk_mem_alloc.h"
#include <vulkan/vulkan.h>

#include <cassert>
#include <fstream>
#include <string>

namespace inexor::vulkan_renderer {

class Shader {
private:
    std::string name;

    VkShaderStageFlagBits type;

    std::string entry_point;

    VkDevice device;

    VmaAllocator vma_allocator;

    VkShaderModule shader_module;

public:
    // To make this class uncopiable and move only, we implement the "rule of 5".

    VkShaderModule get_module() const {
        return shader_module;
    }

    std::string get_name() const {
        return name;
    }

    std::string get_entry_point() const {
        return entry_point;
    }

    VkShaderStageFlagBits get_type() const {
        return type;
    }

    // Delete the copy constructor.
    Shader(const Shader &) = delete;

    // Delete the copy assignment operator.
    Shader &operator=(const Shader &) = delete;

    // Use the move constructor.
    Shader(Shader &&) noexcept = default;

    // Use the move assignment operator.
    Shader &operator=(Shader &&) noexcept = default;

    /// @brief Creates a shader from a SPIR-V file.
    Shader(const VkDevice &device, const VmaAllocator &vma_allocator, const VkShaderStageFlagBits shader_type, const std::string &file_name,
           const std::string internal_shader_name, const std::string shader_entry_point = "main");

    /// @brief Creates a shader from a SPIR-V memory block.
    Shader(const VkDevice &device, const VmaAllocator &vma_allocator, const VkShaderStageFlagBits shader_type, const std::vector<char> &shader_memory,
           const std::string internal_shader_name, const std::string shader_entry_point = "main");

    /// @brief The destructor cleans up all memory allocations
    ~Shader();
};

} // namespace inexor::vulkan_renderer
