#include "inexor/vulkan-renderer/input/input.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::input {

void Input::cursor_position_callback(GLFWwindow * /*window*/, double x_pos, double y_pos) {
    m_kbm_data.set_cursor_pos(x_pos, y_pos);
}

void Input::key_callback(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key < 0 || key > GLFW_KEY_LAST) {
        return;
    }

    switch (action) {
    case GLFW_PRESS:
        m_kbm_data.press_key(key);
        break;
    case GLFW_RELEASE:
        m_kbm_data.release_key(key);
        break;
    default:
        break;
    }
}

void Input::mouse_button_callback(GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
        return;
    }

    switch (action) {
    case GLFW_PRESS:
        m_kbm_data.press_mouse_button(button);
        break;
    case GLFW_RELEASE:
        m_kbm_data.release_mouse_button(button);
        break;
    default:
        break;
    }
}

void Input::mouse_scroll_callback(GLFWwindow * /*window*/, double /*x_offset*/, double y_offset) {
    m_kbm_data.set_mouse_wheel_offset(y_offset);
}

void Input::update() {
    glfwPollEvents();
    update_gamepad_data();
}

void Input::update_gamepad_data() {
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1) != 1) {
        return;
    }
    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state) == 1) {
        for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST; i++) {
            if (state.buttons[i] == 1) {
                m_gamepad_data.press_button(i);
            } else {
                m_gamepad_data.release_button(i);
            }
        }
        for (int i = 0; i < 2; i++) {
            m_gamepad_data.set_joystick_axis(i, state.axes[i]);
            m_gamepad_data.set_joystick_axis(i, state.axes[i + 2], 1);
        }
    }
}

} // namespace inexor::vulkan_renderer::input
