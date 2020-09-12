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

    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_gltf_model_files;

    /// @brief
    /// @param file_name
    void load_toml_configuration_file(const std::string &file_name);

    /// @brief
    VkResult load_textures();

    /// @brief
    VkResult load_shaders();

    /// @brief
    VkResult load_octree_geometry();

    /// @brief
    void update_imgui_overlay();

    /// @brief
    VkResult check_application_specific_features();

    /// @brief
    VkResult update_uniform_buffers();

    /// @brief
    VkResult update_mouse_input();

    // TODO: Refactor!
    double m_cursor_x, m_cursor_y;

public:
    /// @brief
    /// @param argc
    /// @param argv
    Application(int argc, char **argv);

    void run();
};

} // namespace inexor::vulkan_renderer
