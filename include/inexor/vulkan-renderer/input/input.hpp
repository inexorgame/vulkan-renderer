#pragma once

#include <GLFW/glfw3.h>

#include "inexor/vulkan-renderer/input/gamepad_data.hpp"
#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"

namespace inexor::vulkan_renderer::input {
class Input {
private:
    GamepadInputData m_gamepad_data;
    KeyboardMouseInputData m_kbm_data;

public:
    Input() = default;
    Input(const Input &) = delete;
    Input(Input &&) = delete;
    ~Input() = default;

    Input &operator=(const Input &) = delete;
    Input &operator=(Input &&) = delete;

    GamepadInputData &gamepad_data() {
        return m_gamepad_data;
    }
    KeyboardMouseInputData &kbm_data() {
        return m_kbm_data;
    }

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

    void update_gamepad_data();

    void update();
};
} // namespace inexor::vulkan_renderer::input
