#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace {

std::vector<char> read_binary(const std::string &file_name) {
    // Open stream at the end of the file to read it's size.
    std::ifstream file(file_name.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

    if (!file) {
        throw std::runtime_error("Error: Could not open file " + file_name + "!");
    }

    const auto file_size = file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    return buffer;
}

} // namespace

namespace inexor::vulkan_renderer::wrapper {

Shader::Shader(const Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::string &file_name)
    : Shader(device, type, name, read_binary(file_name)) {}

Shader::Shader(const Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::vector<char> &code)
    : m_device(device), m_type(type), m_name(name) {
    assert(device.device());
    assert(!name.empty());
    assert(!code.empty());

    auto shader_module_ci = make_info<VkShaderModuleCreateInfo>();
    shader_module_ci.codeSize = code.size();

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
    // allocator already ensures that the data satisfies the worst case alignment requirements.
    shader_module_ci.pCode = reinterpret_cast<const std::uint32_t *>(code.data()); // NOLINT

    spdlog::debug("Creating shader module {}.", name);
    if (const auto result = vkCreateShaderModule(device.device(), &shader_module_ci, nullptr, &m_shader_module);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateShaderModule failed for shader " + name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_shader_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}

Shader::Shader(Shader &&other) noexcept : m_device(other.m_device) {
    m_type = other.m_type;
    m_name = std::move(other.m_name);
    m_shader_module = std::exchange(other.m_shader_module, nullptr);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_device.device(), m_shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
