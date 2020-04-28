#include "inexor/vulkan-renderer/shader_manager.hpp"

namespace inexor::vulkan_renderer {

VkResult VulkanShaderManager::init(const VkDevice &device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager) {
    assert(device);
    assert(debug_marker_manager);

    spdlog::debug("Initialising shader manager.");

    this->debug_marker_manager = debug_marker_manager;
    this->device = device;

    shader_manager_initialised = true;

    return VK_SUCCESS;
}

VkResult VulkanShaderManager::create_shader_module(const std::vector<char> &SPIRV_shader_bytes, VkShaderModule *shader_module) {
    assert(shader_manager_initialised);
    assert(device);
    assert(debug_marker_manager);
    assert(!SPIRV_shader_bytes.empty());

    spdlog::debug("SPIR-V shader byte size: {}.", SPIRV_shader_bytes.size());

    VkShaderModuleCreateInfo shader_create_info = {};

    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.pNext = nullptr;
    shader_create_info.flags = 0;
    shader_create_info.codeSize = SPIRV_shader_bytes.size();

    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment requirements of std::uint32_t.
    // Lucky for us, the data is stored in an std::vector where the default allocator already ensures that the data satisfies the worst case alignment
    // requirements.
    shader_create_info.pCode = reinterpret_cast<const std::uint32_t *>(SPIRV_shader_bytes.data());

    VkResult result = vkCreateShaderModule(device, &shader_create_info, nullptr, shader_module);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanShaderManager::create_shader_from_memory(const std::string &internal_shader_name, const VkShaderStageFlagBits &shader_type,
                                                        const std::vector<char> &SPIRV_shader_bytes, const std::string &shader_entry_point) {
    assert(device);
    assert(debug_marker_manager);
    assert(shader_manager_initialised);
    assert(!internal_shader_name.empty());
    assert(!SPIRV_shader_bytes.empty());

    spdlog::debug("Creating shader '{}' from memory.", internal_shader_name.c_str());

    std::shared_ptr<Shader> new_shader = std::make_shared<Shader>();

    new_shader->type = shader_type;
    new_shader->name = internal_shader_name;
    new_shader->entry_name = shader_entry_point;

    // Create the shader module from the SPIR-V byte buffer.
    VkShaderModule shader_module;
    VkResult result = create_shader_module(SPIRV_shader_bytes, &shader_module);
    if (result != VK_SUCCESS) {
        vulkan_error_check(result);
        return result;
    }

    // Store the generated shader module.
    new_shader->module = shader_module;

    // Call template base class method.
    add_entry(internal_shader_name, new_shader);

    return VK_SUCCESS;
}

VkResult VulkanShaderManager::create_shader_from_file(const VkShaderStageFlagBits &shader_type, const std::string &SPIRV_shader_file_name,
                                                      const std::string &internal_shader_name, const std::string &shader_entry_point) {
    assert(device);
    assert(debug_marker_manager);
    assert(shader_manager_initialised);
    assert(!SPIRV_shader_file_name.empty());

    spdlog::debug("Creating shader '{}' from file.", SPIRV_shader_file_name.c_str());

    std::shared_ptr<Shader> new_shader = std::make_shared<Shader>();

    // Load the fragment shader into memory.
    new_shader->load_file(SPIRV_shader_file_name);

    VkShaderModule new_shader_module;

    // Create a Vulkan shader module.
    VkResult result = create_shader_module(new_shader->get_file_data(), &new_shader_module);
    if (result != VK_SUCCESS) {
        vulkan_error_check(result);
        return result;
    }

    std::string shader_debug_marker_name = "Shader module '" + SPIRV_shader_file_name + "'.";

    // Give this shader an appropriate name.
    debug_marker_manager->set_object_name(device, (std::uint64_t)(new_shader_module), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, internal_shader_name.c_str());

    // Store the generated shader in memory.
    new_shader->entry_name = shader_entry_point;
    new_shader->name = internal_shader_name;
    new_shader->type = shader_type;
    new_shader->module = new_shader_module;

    // Call template base class method.
    add_entry(internal_shader_name, new_shader);

    return VK_SUCCESS;
}

void VulkanShaderManager::shutdown_shaders() {
    assert(device);
    assert(shader_manager_initialised);

    spdlog::debug("Shutting down shader manager.");

    // Call template base class method.
    auto shaders = get_all_values();

    for (const auto &shader : shaders) {
        spdlog::debug("Destroying shader module '{}'.", shader->name);

        vkDestroyShaderModule(device, shader->module, nullptr);
    }

    delete_all_entries();
}

std::vector<std::shared_ptr<Shader>> VulkanShaderManager::get_all_shaders() {
    assert(shader_manager_initialised);

    // Call template base class method.
    return get_all_values();
}

} // namespace inexor::vulkan_renderer
