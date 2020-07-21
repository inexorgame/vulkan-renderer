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

Shader::Shader(VkDevice device, const VkShaderStageFlagBits type, const std::string &name, const std::string &file_name,
               const std::string &entry_point)
    : Shader(device, type, name, read_binary(file_name), entry_point) {}

Shader::Shader(Shader &&shader) noexcept
    : device(shader.device), type(shader.type), name(std::move(shader.name)),
      entry_point(std::move(shader.entry_point)), shader_module(std::exchange(shader.shader_module, nullptr)) {}

Shader::Shader(VkDevice device, const VkShaderStageFlagBits type, const std::string &name,
               const std::vector<char> &code, const std::string &entry_point)
    : device(device), type(type), name(name), entry_point(entry_point) {
    assert(device);
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
    if (vkCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateShaderModule failed for shader " + name + "!");
    }

    // Try to find the Vulkan debug marker function.
    auto *vkDebugMarkerSetObjectNameEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
        vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));

    if (vkDebugMarkerSetObjectNameEXT != nullptr) {
        // Since the function vkDebugMarkerSetObjectNameEXT has been found, we can assign an internal name for
        // debugging.
        auto name_info = make_info<VkDebugMarkerObjectNameInfoEXT>();
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
        name_info.object = reinterpret_cast<std::uint64_t>(shader_module);
        name_info.pObjectName = name.c_str();

        spdlog::debug("Assigning internal name {} to shader module.", name);
        if (vkDebugMarkerSetObjectNameEXT(device, &name_info) != VK_SUCCESS) {
            throw std::runtime_error("Error: vkDebugMarkerSetObjectNameEXT failed for shader " + name + "!");
        }
    }
}

Shader::~Shader() {
    vkDestroyShaderModule(device, shader_module, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
