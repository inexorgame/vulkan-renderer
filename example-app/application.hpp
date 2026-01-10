#pragma once

#include "renderer.hpp"

#include "inexor/vulkan-renderer/input/input.hpp"

namespace inexor::vulkan_renderer::octree {
// Forward declaration
class Cube;
} // namespace inexor::vulkan_renderer::octree

namespace inexor::vulkan_renderer::wrapper::windows {
// Forward declaration
class Window;
} // namespace inexor::vulkan_renderer::wrapper::windows

// Using declarations
using inexor::vulkan_renderer::input::Input;
using inexor::vulkan_renderer::octree::Cube;
using inexor::vulkan_renderer::wrapper::Instance;
using inexor::vulkan_renderer::wrapper::windows::Window;
using inexor::vulkan_renderer::wrapper::windows::WindowSurface;

namespace inexor::example_app {

/// A sample application demonstrating Inexor's vulkan-renderer.
class ExampleApp : public ExampleAppBase {
private:
    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_gltf_model_files;

    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    std::uint32_t m_window_width{0};
    std::uint32_t m_window_height{0};
    Mode m_window_mode{Mode::WINDOWED};
    std::string m_window_title;
    std::vector<GpuTexture> m_textures;

    /// @TODO The TimeStep class can be removed because we have FPSLimiter which delivers the time_passed.
    TimeStep m_stopwatch;
    /// Necessary for taking into account the relative speed of the system's CPU.
    /// @TODO This can also be removed.
    float m_time_passed{0.0f};

    static VkBool32 validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                              void *user_data);

    /// Inexor engine supports a variable number of octrees.
    std::vector<std::shared_ptr<Cube>> m_worlds;

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
    void generate_octree_indices();
    void initialize_spdlog();
    void recreate_swapchain();
    void render_frame();
    void setup_render_graph();

public:
    // A wrapper class for mouse, keyboard, and gamepad/joystick input.
    std::unique_ptr<Input> m_input;

public:
    ExampleApp(int argc, char **argv);

    void run();
};

// Using declarations
using inexor::vulkan_renderer::tools::FPSLimiter;
using inexor::vulkan_renderer::tools::InexorException;
using inexor::vulkan_renderer::tools::VulkanException;

} // namespace inexor::example_app
