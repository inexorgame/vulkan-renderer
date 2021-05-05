#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"

namespace inexor::vulkan_renderer::input {

void KeyboardMouseInputData::press_key(const std::int32_t key) {
    m_pressed_keys[key] = true;
    keyboard_updated = true;
}

void KeyboardMouseInputData::release_key(const std::int32_t key) {
    m_pressed_keys[key] = false;
    keyboard_updated = true;
}

void KeyboardMouseInputData::press_mouse_button(const std::int32_t button) {
    m_pressed_mouse_buttons[button] = true;
    mouse_buttons_updated = true;
}

void KeyboardMouseInputData::release_mouse_button(const std::int32_t button) {
    m_pressed_mouse_buttons[button] = false;
    mouse_buttons_updated = true;
}

void KeyboardMouseInputData::set_cursor_pos(const double cursor_pos_x, const double cursor_pos_y) {
    m_current_cursor_pos[0] = static_cast<std::int64_t>(cursor_pos_x);
    m_current_cursor_pos[1] = static_cast<std::int64_t>(cursor_pos_y);
}

std::array<std::int64_t, 2> KeyboardMouseInputData::get_cursor_pos() const {
    return m_current_cursor_pos;
}

bool KeyboardMouseInputData::is_mouse_button_pressed(std::int32_t mouse_button) {
    return m_pressed_mouse_buttons[mouse_button];
}

bool KeyboardMouseInputData::is_key_pressed(std::int32_t key) {
    return m_pressed_keys[key];
}

std::array<double, 2> KeyboardMouseInputData::calculate_cursor_position_delta() {
    // Calculate the change in cursor position in x- and y-axis.
    const std::array<double, 2> m_cursor_pos_delta{
        static_cast<double>(m_current_cursor_pos[0]) - static_cast<double>(m_previous_cursor_pos[0]),
        static_cast<double>(m_current_cursor_pos[1]) - static_cast<double>(m_previous_cursor_pos[1])};

    m_previous_cursor_pos = m_current_cursor_pos;

    return m_cursor_pos_delta;
}

[[nodiscard]] bool KeyboardMouseInputData::was_key_pressed_once(const std::int32_t key) {
    if (!keyboard_updated) {
        return false;
    }
    if (m_pressed_keys[key]) {
        m_pressed_keys[key] = false;
        return true;
    }
    return false;
}

[[nodiscard]] bool KeyboardMouseInputData::was_mouse_button_pressed_once(const std::int32_t mouse_button) {
    if (!mouse_buttons_updated) {
        return false;
    }
    if (m_pressed_mouse_buttons[mouse_button]) {
        m_pressed_mouse_buttons[mouse_button] = false;
        return true;
    }
    return false;
}

} // namespace inexor::vulkan_renderer::input
