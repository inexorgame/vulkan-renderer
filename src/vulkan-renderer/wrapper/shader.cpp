#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/file.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Shader::Shader(const Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::string &file_name, const std::string &entry_point)
    : Shader(device, type, name, tools::read_file_binary_data(file_name), entry_point) {}

Shader::Shader(const Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::vector<char> &code, const std::string &entry_point)
    : m_device(device), m_type(type), m_name(name), m_entry_point(entry_point) {
    assert(device.device());
    assert(!name.empty());
    assert(!code.empty());
    assert(!entry_point.empty());

    const auto shader_module_ci = make_info<VkShaderModuleCreateInfo>({
        .codeSize = code.size(),
        // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
        // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
        // allocator already ensures that the data satisfies the worst case alignment requirements.
        .pCode = reinterpret_cast<const std::uint32_t *>(code.data()), // NOLINT
    });

    if (const auto result = vkCreateShaderModule(m_device.device(), &shader_module_ci, nullptr, &m_shader_module);
        result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkCreateShaderModule failed for shader module " + name + "!", result);
    }

    m_device.set_debug_marker_name(&m_shader_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}

Shader::Shader(Shader &&other) noexcept : m_device(other.m_device) {
    m_type = other.m_type;
    m_name = std::move(other.m_name);
    m_entry_point = std::move(other.m_entry_point);
    m_shader_module = std::exchange(other.m_shader_module, nullptr);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_device.device(), m_shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
