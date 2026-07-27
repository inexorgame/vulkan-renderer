#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/file.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Shader::Shader(const Device &device, const VkShaderStageFlagBits shader_stage, const std::string &shader_file_name,
               const std::string &entry_point)
    : m_device(device), m_shader_stage(shader_stage), m_name(shader_file_name), m_entry_point(entry_point) {
    if (shader_file_name.empty()) {
        throw std::invalid_argument("Error: Parameter 'shader_file_name' is an empty string!");
    }

    const auto shader_code = tools::read_file_binary_data(shader_file_name);
    if (shader_code.empty()) {
        throw std::runtime_error("Error: read_file_binary_data(shader_file_name) returned an empty array!");
    }

    const auto shader_module_ci = tools::make_info<VkShaderModuleCreateInfo>({
        .codeSize = shader_code.size(),
        // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
        // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
        // allocator already ensures that the data satisfies the worst case alignment requirements.
        .pCode = reinterpret_cast<const std::uint32_t *>(shader_code.data()), // NOLINT
    });

    if (const auto result = vkCreateShaderModule(m_device.device(), &shader_module_ci, nullptr, &m_shader_module);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateShaderModule failed!", result, m_name);
    }
    m_device.set_debug_name(m_shader_module, m_name);
}

Shader::Shader(Shader &&other) noexcept : m_device(other.m_device) {
    m_shader_stage = other.m_shader_stage;
    m_name = std::move(other.m_name);
    m_entry_point = std::move(other.m_entry_point);
    m_shader_module = std::exchange(other.m_shader_module, nullptr);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_device.device(), m_shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
