#include "inexor/vulkan-renderer/render-graph/shader.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Shader::Shader(const Device &device, std::string name, const VkShaderStageFlagBits type, std::string file_name)
    : m_device(device), m_name(std::move(name)), m_shader_stage(type), m_file_name(file_name) {
    if (m_name.empty()) {
        throw std::runtime_error("[Shader::Shader] Error: Parameter 'name' is empty!");
    }
    // Open the file stream at the end of the file to read file size
    std::ifstream shader_file(file_name.c_str(), std::ios::ate | std::ios::binary | std::ios::in);
    if (!shader_file) {
        throw std::runtime_error("[Shader::Shader] Error: Could not open shader file " + file_name + "!");
    }

    // Read the size of the file
    const auto file_size = shader_file.tellg();
    // Create a vector of char (bytes) with the size of the shader file
    // After the creation of the shader module, this is no longer needed, so it is not a problem that its object
    // lifetime ends with the constructor's stack
    std::vector<char> shader_code(file_size);
    // Set the file read position to the beginning of the file
    shader_file.seekg(0);
    // Read the entire shader file into memory
    shader_file.read(shader_code.data(), file_size);

    const auto shader_module_ci = wrapper::make_info<VkShaderModuleCreateInfo>({
        .codeSize = shader_code.size(),
        // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
        // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the
        // default allocator already ensures that the data satisfies the worst case alignment requirements.
        .pCode = reinterpret_cast<const std::uint32_t *>(shader_code.data()), // NOLINT
    });

    if (const auto result = vkCreateShaderModule(m_device.device(), &shader_module_ci, nullptr, &m_shader_module);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateShaderModule failed for shader " + file_name + "!", result);
    }
    m_device.set_debug_name(m_shader_module, file_name);
}

Shader::Shader(Shader &&other) noexcept : m_device(other.m_device) {
    m_shader_stage = other.m_shader_stage;
    m_name = std::move(other.m_name);
    m_file_name = std::move(other.m_file_name);
    m_shader_module = std::exchange(other.m_shader_module, VK_NULL_HANDLE);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_device.device(), m_shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::render_graph
