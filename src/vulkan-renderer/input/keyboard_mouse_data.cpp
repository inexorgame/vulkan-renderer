#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"

#include <cassert>
#include <mutex>

namespace inexor::vulkan_renderer::input {

glm::dvec2 KeyboardMouseInputData::calculate_cursor_position_delta() {
    std::scoped_lock lock(m_input_mutex);
    // Calculate the change in cursor position in x- and y-axis.
    auto m_cursor_pos_delta = m_current_cursor_pos - m_previous_cursor_pos;
    m_previous_cursor_pos = m_current_cursor_pos;
    return m_cursor_pos_delta;
}

glm::ivec2 KeyboardMouseInputData::get_cursor_pos() const {
    std::scoped_lock lock(m_input_mutex);
    return m_current_cursor_pos;
}

[[nodiscard]] double KeyboardMouseInputData::get_mouse_wheel_offset() const {
    std::scoped_lock lock(m_input_mutex);
    return m_mouse_wheel_offset;
}

bool KeyboardMouseInputData::is_key_pressed(const std::int32_t key) const {
    std::scoped_lock lock(m_input_mutex);
    return m_key_states.at(key);
}

bool KeyboardMouseInputData::is_mouse_button_pressed(const std::int32_t button) const {
    std::scoped_lock lock(m_input_mutex);
    return m_mouse_button_states.at(button);
}

void KeyboardMouseInputData::press_key(const std::int32_t key) {
    std::scoped_lock lock(m_input_mutex);
    m_key_states.at(key) = true;
    m_keyboard_updated = true;
}

void KeyboardMouseInputData::press_mouse_button(const std::int32_t button) {
    std::scoped_lock lock(m_input_mutex);
    m_mouse_button_states.at(button) = true;
    m_mouse_buttons_updated = true;
}

void KeyboardMouseInputData::release_mouse_button(const std::int32_t button) {
    std::scoped_lock lock(m_input_mutex);
    m_mouse_button_states.at(button) = false;
    m_mouse_buttons_updated = true;
}

void KeyboardMouseInputData::release_key(const std::int32_t key) {
    std::scoped_lock lock(m_input_mutex);
    m_key_states.at(key) = false;
    m_keyboard_updated = true;
}

void KeyboardMouseInputData::set_cursor_pos(const double pos_x, const double pos_y) {
    std::scoped_lock lock(m_input_mutex);
    m_current_cursor_pos = {pos_x, pos_y};
}

void KeyboardMouseInputData::set_mouse_wheel_offset(double y_offset) {
    std::scoped_lock lock(m_input_mutex);
    m_mouse_wheel_offset = y_offset;
}

bool KeyboardMouseInputData::was_key_pressed_once(const std::int32_t key) {
    std::scoped_lock lock(m_input_mutex);
    if (!m_key_states.at(key) || !m_keyboard_updated) {
        return false;
    }

    m_key_states.at(key) = false;
    return true;
}

bool KeyboardMouseInputData::was_mouse_button_pressed_once(const std::int32_t button) {
    std::scoped_lock lock(m_input_mutex);
    if (!m_mouse_button_states.at(button) || !m_mouse_buttons_updated) {
        return false;
    }
    m_mouse_button_states.at(button) = false;
    return true;
}

} // namespace inexor::vulkan_renderer::input
