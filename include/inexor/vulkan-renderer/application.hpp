#pragma once

#include "inexor/vulkan-renderer/renderer.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

class Application : public VulkanRenderer {
private:
    std::string m_application_name;

    std::string m_engine_name;

    std::uint32_t m_application_version{};

    std::uint32_t m_engine_version{};

    // TODO: Refactor into a manger class.
    struct ShaderSetup {
        VkShaderStageFlagBits shader_type;
        std::string shader_file_name;
    };

    std::vector<std::string> m_vertex_shader_files;

    std::vector<std::string> m_fragment_shader_files;

    std::vector<std::string> m_texture_files;

    std::vector<std::string> m_shader_files;

    std::vector<std::string> m_gltf_model_files;

private:
    /// @brief Loads the configuration of the renderer from a TOML configuration file.
    /// @brief file_name [in] The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    VkResult load_toml_configuration_file(const std::string &file_name);

    VkResult load_textures();

    VkResult load_shaders();

    VkResult load_octree_geometry();

    VkResult check_application_specific_features();

    /// @brief Implementation of the uniform buffer update method.
    /// @param current_image [in] The current image index.
    VkResult update_uniform_buffers();

    VkResult update_mouse_input();

    // TODO: Refactor!
    double m_cursor_x, m_cursor_y;

public:
    Application(int argc, char **argv);

    void run();
};

} // namespace inexor::vulkan_renderer
