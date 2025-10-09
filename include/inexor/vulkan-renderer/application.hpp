#pragma once

#include "inexor/vulkan-renderer/input/input.hpp"
#include "inexor/vulkan-renderer/renderer.hpp"
#include "inexor/vulkan-renderer/world/collision_query.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

// Forward declarations
namespace inexor::vulkan_renderer::input {
class KeyboardMouseInputData;
} // namespace inexor::vulkan_renderer::input

namespace inexor::vulkan_renderer {

class Application : public VulkanRenderer {
    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_gltf_model_files;

    std::unique_ptr<input::Input> m_input;

    bool m_enable_validation_layers{true};
    /// Inexor engine supports a variable number of octrees.
    std::vector<std::shared_ptr<world::Cube>> m_worlds;

    // If the user specified command line argument "--stop-on-validation-message", the program will call
    // std::abort(); after reporting a validation layer (error) message.
    bool m_stop_on_validation_message{false};

    /// @brief Load the configuration of the renderer from a TOML configuration file.
    /// @brief file_name The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    void load_toml_configuration_file(const std::string &file_name);
    void load_textures();
    void load_shaders();
    /// @param initialize Initialize worlds with a fixed seed, which is useful for benchmarking and testing
    void load_octree_geometry(bool initialize);
    void setup_vulkan_debug_callback();
    void setup_window_and_input_callbacks();
    void update_imgui_overlay();
    void update_uniform_buffers();
    /// Use the camera's position and view direction vector to check for ray-octree collisions with all octrees.
    void check_octree_collisions();
    void process_input();

public:
    Application(int argc, char **argv);

    void run();
};

} // namespace inexor::vulkan_renderer
