#pragma once

#include "inexor/vulkan-renderer/input/input.hpp"
#include "inexor/vulkan-renderer/octree/collision_query.hpp"
#include "inexor/vulkan-renderer/octree/cube.hpp"
#include "inexor/vulkan-renderer/renderer.hpp"

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

    static VkBool32 validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                              void *user_data);

    /// Inexor engine supports a variable number of octrees.
    std::vector<std::shared_ptr<octree::Cube>> m_worlds;

    /// @brief Load the configuration of the renderer from a TOML configuration file.
    /// @brief file_name The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    void load_toml_configuration_file(const std::string &file_name);
    void load_shaders();
    /// @param initialize Initialize worlds with a fixed seed, which is useful for benchmarking and testing
    void load_octree_geometry(bool initialize);
    void setup_window_and_input_callbacks();
    void update_imgui_overlay();
    void update_uniform_buffers();
    /// Use the camera's position and view direction vector to check for ray-octree collisions with all octrees.
    void check_octree_collisions();
    void process_input();
    void initialize_spdlog();

public:
    Application(int argc, char **argv);

    void run();
};

// Using declarations
using input::Input;
using tools::InexorException;
using tools::VulkanException;
using wrapper::Instance;
using wrapper::window::Window;
using wrapper::window::WindowSurface;

} // namespace inexor::vulkan_renderer
