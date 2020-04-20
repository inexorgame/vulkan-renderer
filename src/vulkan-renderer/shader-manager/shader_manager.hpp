#pragma once

#include "vulkan-renderer/class-templates/manager_template.hpp"
#include "vulkan-renderer/debug-marker/debug_marker_manager.hpp"
#include "vulkan-renderer/error-handling/error_handling.hpp"
#include "vulkan-renderer/shader-manager/shader_manager.hpp"
#include "vulkan-renderer/shader/shader.hpp"

#include <string>
#include <vector>

#include <spdlog/spdlog.h>

namespace inexor {
namespace vulkan_renderer {

/// @class VulkanShaderManager
/// @brief A class for managing SPIR-V shaders.
class VulkanShaderManager : public ManagerClassTemplate<InexorShader> {
public:
    VulkanShaderManager() = default;

    ~VulkanShaderManager() = default;

private:
    bool shader_manager_initialised;

    std::mutex shader_manager_mutex;

    VkDevice device;

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

private:
    /// @brief Creates a shader module.
    /// @param vulkan_device The Vulkan device handle.
    /// @param SPIRV_shader_bytes [in] The binary data of the shader.
    /// @param shader_module [out] The shader module.
    /// @note The buffer with the SPIR-V code can be freed immediately after the shader module was created.
    VkResult create_shader_module(const std::vector<char> &SPIRV_shader_bytes, VkShaderModule *shader_module);

public:
    /// @param debug_marker_manager_instance The VulkanDebugMarkerManager instance.
    VkResult init(const VkDevice &device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager);

    /// @brief Creates a new shader from SPIR-V byte buffer.
    /// @param vulkan_device The Vulkan device.
    /// @param shader_type The shader type (e.g. vertex-, fragment-, tesselation-, geometry-, compute-shader)
    /// @param SPIRV_shader_bytes The byte buffer of the SPIR-V shader file.
    /// @param shader_name The name you want to give to the shader.
    /// @param shader_entry_point The entry point of the shader, usually "main".
    /// @return True is shader creation succeeded, false otherwise.
    VkResult create_shader_from_memory(const std::string &internal_shader_name, const VkShaderStageFlagBits &shader_type,
                                       const std::vector<char> &SPIRV_shader_bytes, const std::string &shader_entry_point);

    /// @brief Creates a new shader from a file on the hard drive.
    /// @param vulkan_device The Vulkan device.
    /// @param shader_type The shader type (e.g. vertex-shader, fragment-shader, tesselation-shader, geometry-shader, compute-shader)
    /// @param SPIRV_shader_file_name The file name of the SPIR-V shader file.
    /// @param shader_name The name you want to give to the shader.
    /// @param shader_entry_point The entry point of the shader, usually "main".
    /// @return True is shader creation succeeded, false otherwise.
    VkResult create_shader_from_file(const VkShaderStageFlagBits &shader_type, const std::string &SPIRV_shader_file_name,
                                     const std::string &internal_shader_name, const std::string &shader_entry_point);

    /// @brief Returns all the shaders which have been loaded.
    /// @return A const vector of InexorVulkanShader instances.
    std::vector<std::shared_ptr<InexorShader>> get_all_shaders();

    /// @brief Destroys all shader objects.
    void shutdown_shaders();
};

}; // namespace vulkan_renderer
}; // namespace inexor
