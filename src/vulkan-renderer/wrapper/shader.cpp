#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>
#include <spirv/unified1/spirv.h>

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

VkShaderStageFlagBits shader_stage(SpvExecutionModel execution_model) {
    switch (execution_model) {
    case SpvExecutionModelVertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case SpvExecutionModelFragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case SpvExecutionModelGLCompute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        assert(false);
    }
}

} // namespace

namespace inexor::vulkan_renderer::wrapper {

Shader::Shader(const Device &device, const std::string &name, const std::string &file_name)
    : Shader(device, name, read_binary(file_name)) {}

Shader::Shader(const Device &device, const std::string &name, std::vector<char> &&binary)
    : m_device(device), m_name(name) {
    assert(!name.empty());
    assert(!binary.empty());

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
    // allocator already ensures that the data satisfies the worst case alignment requirements.
    const auto *code = reinterpret_cast<const std::uint32_t *>(binary.data()); // NOLINT
    auto shader_module_ci = make_info<VkShaderModuleCreateInfo>();
    shader_module_ci.codeSize = binary.size();
    shader_module_ci.pCode = code;

    spdlog::debug("Creating shader module {}", name);
    if (const auto result = vkCreateShaderModule(device.device(), &shader_module_ci, nullptr, &m_module);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateShaderModule failed for shader " + name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);

    // Parse SPIR-V to extract the shader stage.
    assert(code[0] == SpvMagicNumber);
    const auto *inst = code + 5;
    while (inst != code + (binary.size() / 4)) {
        // Each instruction starts with a dword with the upper 16 bits holding the total number of words in the
        // instruction and the lower 16 bits holding the opcode.
        std::uint16_t opcode = (inst[0] >> 0u) & 0xffffu;
        std::uint16_t word_count = (inst[0] >> 16u) & 0xffffu;
        if (opcode == SpvOpEntryPoint) {
            assert(word_count >= 2);
            m_stage = shader_stage(static_cast<SpvExecutionModel>(inst[1]));
        }
        inst += word_count;
    }
}

Shader::Shader(Shader &&other) noexcept : m_device(other.m_device), m_stage(other.m_stage) {
    m_name = std::move(other.m_name);
    m_module = std::exchange(other.m_module, nullptr);
}

Shader::~Shader() {
    vkDestroyShaderModule(m_device.device(), m_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
