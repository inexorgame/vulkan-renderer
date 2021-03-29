#pragma once

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/renderer.hpp"
#include "inexor/vulkan-renderer/world/collision_query.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// forward declarations
namespace inexor::vulkan_renderer::input {
class KeyboardMouseInputData;
} // namespace inexor::vulkan_renderer::input

namespace inexor::vulkan_renderer {

class Application : public VulkanRenderer {
    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_gltf_model_files;

    std::unique_ptr<input::KeyboardMouseInputData> m_input_data;

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
    void load_octree_geometry();
    void setup_vulkan_debug_callback();
    void setup_window_and_input_callbacks();
    void update_imgui_overlay();
    void check_application_specific_features();
    void update_uniform_buffers();
    /// Use the camera's position and view direction vector to check for ray-octree collisions with all octrees.
    void check_octree_collisions();
    void process_mouse_input();

public:
    Application(int argc, char **argv);

    /// @brief Call glfwSetKeyCallback.
    /// @param window The window that received the event.
    /// @param key The keyboard key that was pressed or released.
    /// @param scancode The system-specific scancode of the key.
    /// @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
    /// @param mods Bit field describing which modifier keys were held down.
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    /// @brief Call glfwSetCursorPosCallback.
    /// @param window The window that received the event.
    /// @param x_pos The new x-coordinate, in screen coordinates, of the cursor.
    /// @param y_pos The new y-coordinate, in screen coordinates, of the cursor.
    void cursor_position_callback(GLFWwindow *window, double x_pos, double y_pos);

    /// @brief Call glfwSetMouseButtonCallback.
    /// @param window The window that received the event.
    /// @param button The mouse button that was pressed or released.
    /// @param action One of GLFW_PRESS or GLFW_RELEASE.
    /// @param mods Bit field describing which modifier keys were held down.
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    /// @brief Call camera's process_mouse_scroll method.
    /// @param window The window that received the event.
    /// @param x_offset The change of x-offset of the mouse wheel.
    /// @param y_offset The change of y-offset of the mouse wheel.
    void mouse_scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

    void run();
};

} // namespace inexor::vulkan_renderer
