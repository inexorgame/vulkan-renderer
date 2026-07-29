#pragma once

#include "renderer.hpp"

namespace inexor::vulkan_renderer::octree {
// Forward declaration
class Cube;
} // namespace inexor::vulkan_renderer::octree

namespace inexor::vulkan_renderer::render_modules::octree {
// Forward declaration
class OctreeVertex;
} // namespace inexor::vulkan_renderer::render_modules::octree

namespace inexor::vulkan_renderer::input {
// Forward declaration
class Input;
} // namespace inexor::vulkan_renderer::input

namespace inexor::vulkan_renderer::wrapper::windows {
// Forward declaration
enum class WindowMode;
} // namespace inexor::vulkan_renderer::wrapper::windows

namespace inexor::example_app {

// Using declarations
using vulkan_renderer::input::Input;
using vulkan_renderer::octree::Cube;
using vulkan_renderer::render_graph::TextureUsage;
using vulkan_renderer::render_modules::octree::OctreeVertex;
using vulkan_renderer::tools::CameraMovement;
using vulkan_renderer::tools::CameraType;
using vulkan_renderer::tools::FPSLimiter;
using vulkan_renderer::tools::InexorException;
using vulkan_renderer::tools::VulkanException;
using vulkan_renderer::wrapper::core::Instance;
using vulkan_renderer::wrapper::descriptors::DescriptorSetLayoutBuilder;
using vulkan_renderer::wrapper::descriptors::WriteDescriptorSetBuilder;
using vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder;
using vulkan_renderer::wrapper::windows::WindowMode;

/// A sample application demonstrating Inexor's vulkan-renderer.
class ExampleApp : public ExampleAppBase {
private:
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    std::uint32_t m_window_width{0};
    std::uint32_t m_window_height{0};
    WindowMode m_window_mode;
    std::string m_window_title;
    bool m_no_cmd_buf_cache{false};

    std::vector<OctreeVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;

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
    /// @param initialize Initialize worlds with a fixed seed, which is useful for benchmarking and testing
    void load_octree_geometry(bool initialize);
    void setup_window_and_input_callbacks();
    void update_imgui_overlay();
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
    ~ExampleApp();

    void run();
};

} // namespace inexor::example_app
