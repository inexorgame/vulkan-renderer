#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

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

Shader::Shader(wrapper::Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::string &file_name, const std::string &entry_point)
    : Shader(device, type, name, read_binary(file_name), entry_point) {}

Shader::Shader(wrapper::Device &device, const VkShaderStageFlagBits type, const std::string &name,
               const std::vector<char> &code, const std::string &entry_point)
    : m_device(device), m_type(type), m_name(name), m_entry_point(entry_point) {
    assert(device.device());
    assert(!name.empty());
    assert(!code.empty());
    assert(!entry_point.empty());

    auto shader_module_ci = make_info<VkShaderModuleCreateInfo>();
    shader_module_ci.codeSize = code.size();

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
    // allocator already ensures that the data satisfies the worst case alignment requirements.
    shader_module_ci.pCode = reinterpret_cast<const std::uint32_t *>(code.data());

    spdlog::debug("Creating shader module {}.", name);
    if (vkCreateShaderModule(device.device(), &shader_module_ci, nullptr, &m_shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateShaderModule failed for shader " + name + "!");
    }

#ifndef NDEBUG
    // Assign an internal name using Vulkan debug markers.
    m_device.set_object_name((std::uint64_t)m_shader_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
#endif
}

Shader::Shader(Shader &&other) noexcept
    : m_device(std::move(other.m_device)), m_type(other.m_type), m_name(std::move(other.m_name)),
      m_entry_point(std::move(other.m_entry_point)), m_shader_module(std::exchange(other.m_shader_module, nullptr)) {}

Shader::~Shader() {
    spdlog::trace("Destroying shader module {}.", m_name);
    vkDestroyShaderModule(m_device.device(), m_shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
