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
    std::string m_application_name;
    std::string m_engine_name;

    std::uint32_t m_application_version{};
    std::uint32_t m_engine_version{};

    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_gltf_model_files;

    /// @brief Load the configuration of the renderer from a TOML configuration file.
    /// @brief file_name The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    void load_toml_configuration_file(const std::string &file_name);
    void load_textures();
    void load_shaders();
    void load_octree_geometry();
    void update_imgui_overlay();
    void check_application_specific_features();
    void update_uniform_buffers();
    void update_mouse_input();

    // TODO: Refactor!
    double m_cursor_x, m_cursor_y;

public:
    Application(int argc, char **argv);

    void run();
};

} // namespace inexor::vulkan_renderer
