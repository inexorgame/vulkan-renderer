#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"

#include <cassert>
#include <mutex>

namespace inexor::vulkan_renderer::input {

void KeyboardMouseInputData::press_key(const std::int32_t key) {
    assert(key < GLFW_KEY_LAST);
    std::scoped_lock lock(m_input_mutex);
    m_pressed_keys[key] = true;
    m_keyboard_updated = true;
}

void KeyboardMouseInputData::release_key(const std::int32_t key) {
    assert(key < GLFW_KEY_LAST);
    std::scoped_lock lock(m_input_mutex);
    m_pressed_keys[key] = false;
    m_keyboard_updated = true;
}

bool KeyboardMouseInputData::is_key_pressed(const std::int32_t key) const {
    assert(key < GLFW_KEY_LAST);
    std::shared_lock lock(m_input_mutex);
    return m_pressed_keys[key];
}
bool KeyboardMouseInputData::was_key_pressed_once(const std::int32_t key) {
    assert(key < GLFW_KEY_LAST);
    std::scoped_lock lock(m_input_mutex);
    if (!m_pressed_keys[key] || !m_keyboard_updated) {
        return false;
    }
    m_pressed_keys[key] = false;
    return true;
}

void KeyboardMouseInputData::press_mouse_button(const std::int32_t button) {
    assert(button < GLFW_MOUSE_BUTTON_LAST);
    std::scoped_lock lock(m_input_mutex);
    m_pressed_mouse_buttons[button] = true;
    m_mouse_buttons_updated = true;
}

void KeyboardMouseInputData::release_mouse_button(const std::int32_t button) {
    assert(button < GLFW_MOUSE_BUTTON_LAST);
    std::scoped_lock lock(m_input_mutex);
    m_pressed_mouse_buttons[button] = false;
    m_mouse_buttons_updated = true;
}

bool KeyboardMouseInputData::is_mouse_button_pressed(const std::int32_t button) const {
    assert(button < GLFW_MOUSE_BUTTON_LAST);
    std::shared_lock lock(m_input_mutex);
    return m_pressed_mouse_buttons[button];
}

bool KeyboardMouseInputData::was_mouse_button_pressed_once(const std::int32_t button) {
    assert(button < GLFW_MOUSE_BUTTON_LAST);
    std::scoped_lock lock(m_input_mutex);
    if (!m_pressed_mouse_buttons[button] || !m_mouse_buttons_updated) {
        return false;
    }
    m_pressed_mouse_buttons[button] = false;
    return true;
}

void KeyboardMouseInputData::set_cursor_pos(const double pos_x, const double pos_y) {
    std::scoped_lock lock(m_input_mutex);
    m_current_cursor_pos[0] = static_cast<std::int64_t>(pos_x);
    m_current_cursor_pos[1] = static_cast<std::int64_t>(pos_y);
}

std::array<std::int64_t, 2> KeyboardMouseInputData::get_cursor_pos() const {
    std::shared_lock lock(m_input_mutex);
    return m_current_cursor_pos;
}

std::array<double, 2> KeyboardMouseInputData::calculate_cursor_position_delta() {
    // Calculate the change in cursor position in x- and y-axis.
    const std::array m_cursor_pos_delta{
        static_cast<double>(m_current_cursor_pos[0]) - static_cast<double>(m_previous_cursor_pos[0]),
        static_cast<double>(m_current_cursor_pos[1]) - static_cast<double>(m_previous_cursor_pos[1])};

    std::scoped_lock lock(m_input_mutex);
    m_previous_cursor_pos = m_current_cursor_pos;

    return m_cursor_pos_delta;
}

} // namespace inexor::vulkan_renderer::input
