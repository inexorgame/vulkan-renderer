#pragma once

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/renderer.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// forward declarations
namespace inexor::vulkan_renderer::input {
class KeyboardMouseInputData;
}

namespace inexor::vulkan_renderer {

class Application : public VulkanRenderer {
    std::string m_application_name;
    std::string m_engine_name;

    std::uint32_t m_application_version;
    std::uint32_t m_engine_version;

    std::vector<std::string> m_vertex_shader_files;
    std::vector<std::string> m_fragment_shader_files;
    std::vector<std::string> m_texture_files;
    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_gltf_model_files;

    std::unique_ptr<input::KeyboardMouseInputData> m_input_data;

    // If the user specified command line argument "--stop-on-validation-message", the program will call std::abort();
    // after reporting a validation layer (error) message.
    bool m_stop_on_validation_message = false;

    /// @brief Load the configuration of the renderer from a TOML configuration file.
    /// @brief file_name The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    void load_toml_configuration_file(const std::string &file_name);
    void load_textures();
    void load_shaders();
    void load_octree_geometry();
    void setup_window_and_input_callbacks();
    void update_imgui_overlay();
    void check_application_specific_features();
    void update_uniform_buffers();
    void process_mouse_input();
    // TODO: Implement a method for processing keyboard input.

public:
    Application(int argc, char **argv);

    /// @brief Call glfwSetFramebufferSizeCallback.
    /// @param window The window whose framebuffer was resized.
    /// @param width The new width, in pixels, of the framebuffer.
    /// @param height The new height, in pixels, of the framebuffer.
    void frame_buffer_resize_callback(GLFWwindow *window, int width, int height);

    /// @brief Call glfwSetKeyCallback.
    /// @param window The window that received the event.
    /// @param key The keyboard key that was pressed or released.
    /// @param scancode The system-specific scancode of the key.
    /// @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
    /// @param mods Bit field describing which modifier keys were held down.
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    /// @brief Call glfwSetCursorPosCallback.
    /// @param window The window that received the event.
    /// @param xpos The new x-coordinate, in screen coordinates, of the cursor.
    /// @param ypos The new y-coordinate, in screen coordinates, of the cursor.
    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

    /// @brief Call glfwSetMouseButtonCallback.
    /// @param window The window that received the event.
    /// @param button The mouse button that was pressed or released.
    /// @param action One of GLFW_PRESS or GLFW_RELEASE.
    /// @param mods Bit field describing which modifier keys were held down.
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    /// @brief Call camera's process_mouse_scroll method.
    /// @param window The window that received the event.
    /// @param xoffset The change of x-offset of the mouse wheel.
    /// @param yoffset The change of y-offset of the mouse wheel.
    void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    void run();
};

} // namespace inexor::vulkan_renderer
