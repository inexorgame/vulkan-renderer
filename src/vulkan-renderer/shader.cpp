#include "inexor/vulkan-renderer/shader.hpp"

namespace inexor::vulkan_renderer {

Shader::Shader(const VkDevice &device, const VmaAllocator &vma_allocator, const VkShaderStageFlagBits shader_type, const std::string &file_name,
               const std::string internal_shader_name, const std::string shader_entry_point) {
    assert(device);
    assert(vma_allocator);
    assert(!file_name.empty());
    assert(!shader_entry_point.empty());
    assert(!internal_shader_name.empty());

    spdlog::debug("Loading SPIR-V shader file {}.", file_name);

    std::vector<char> shader_file_data;

    // Open stream at the end of the file to read it's size.
    std::ifstream spirv_shader_file(file_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (spirv_shader_file.is_open()) {
        spdlog::debug("File {} has been opened.", file_name);

        auto file_size = spirv_shader_file.tellg();

        if (file_size == 0) {
            std::string exception_error_message = "Error: File" + file_name + " is empty!";
            throw std::runtime_error(exception_error_message);
        }

        // Preallocate memory for the file buffer.
        shader_file_data.resize(file_size);

        // Reset the file read position to the beginning of the file.
        spirv_shader_file.seekg(0, std::ios::beg);
        spirv_shader_file.read(shader_file_data.data(), file_size);
        spirv_shader_file.close();

        spdlog::debug("File {} has been closed.", file_name.c_str());
    } else {
        std::string exception_error_message = "Error: Could not open file " + file_name + "!";
        throw std::runtime_error(exception_error_message);
    }

    VkShaderModuleCreateInfo shader_create_info = {};

    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.pNext = nullptr;
    shader_create_info.flags = 0;
    shader_create_info.codeSize = shader_file_data.size();

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
    // allocator already ensures that the data satisfies the worst case alignment requirements.
    shader_create_info.pCode = reinterpret_cast<const std::uint32_t *>(shader_file_data.data());

    spdlog::debug("Creating shader module {} from file {}.", internal_shader_name, file_name);

    if (vkCreateShaderModule(device, &shader_create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateShaderModule failed for shader " + internal_shader_name);
    }

    // Try to find the Vulkan debug marker function.
    auto vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");

    if (vkDebugMarkerSetObjectNameEXT != nullptr) {
        // Since the function vkDebugMarkerSetObjectNameEXT has been found, we can assign an internal name for debugging.
        VkDebugMarkerObjectNameInfoEXT nameInfo = {};

        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
        nameInfo.object = reinterpret_cast<std::uint64_t>(shader_module);
        nameInfo.pObjectName = internal_shader_name.c_str();

        spdlog::debug("Assigning internal name {} to shader module.", internal_shader_name);

        vkDebugMarkerSetObjectNameEXT(device, &nameInfo);
    }

    this->name = internal_shader_name;
    this->type = shader_type;
    this->entry_point = shader_entry_point;
    this->device = device;
    this->vma_allocator = vma_allocator;
    this->shader_module = shader_module;
}

Shader::Shader(const VkDevice &device, const VmaAllocator &vma_allocator, const VkShaderStageFlagBits shader_type, const std::vector<char> &shader_memory,
               const std::string internal_shader_name, const std::string shader_entry_point) {
    assert(device);
    assert(vma_allocator);
    assert(!shader_memory.empty());
    assert(!shader_entry_point.empty());
    assert(!internal_shader_name.empty());

    VkShaderModuleCreateInfo shader_create_info = {};

    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.pNext = nullptr;
    shader_create_info.flags = 0;
    shader_create_info.codeSize = shader_memory.size();

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of std::uint32_t. Lucky for us, the data is stored in an std::vector where the default
    // allocator already ensures that the data satisfies the worst case alignment requirements.
    shader_create_info.pCode = reinterpret_cast<const std::uint32_t *>(shader_memory.data());

    spdlog::debug("Creating shader module {} from memory.", internal_shader_name);

    if (vkCreateShaderModule(device, &shader_create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateShaderModule failed for shader " + internal_shader_name);
    }

    // Try to find the Vulkan debug marker function.
    auto vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");

    if (vkDebugMarkerSetObjectNameEXT != nullptr) {
        // Since the function vkDebugMarkerSetObjectNameEXT has been found, we can assign an internal name for debugging.
        VkDebugMarkerObjectNameInfoEXT nameInfo = {};

        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
        nameInfo.object = reinterpret_cast<std::uint64_t>(shader_module);
        nameInfo.pObjectName = internal_shader_name.c_str();

        spdlog::debug("Assigning internal name {} to shader module.", internal_shader_name);

        vkDebugMarkerSetObjectNameEXT(device, &nameInfo);
    }

    this->name = internal_shader_name;
    this->type = shader_type;
    this->entry_point = shader_entry_point;
    this->device = device;
    this->vma_allocator = vma_allocator;
    this->shader_module = shader_module;
}

Shader::~Shader() {
    // Destroy the shader module in the constructor.
    vkDestroyShaderModule(device, shader_module, nullptr);
}

}; // namespace inexor::vulkan_renderer
