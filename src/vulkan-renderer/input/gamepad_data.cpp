#include "inexor/vulkan-renderer/input/gamepad_data.hpp"

#include <mutex>

namespace inexor::vulkan_renderer::input {

[[nodiscard]] glm::vec2 GamepadInputData::calculate_joystick_axes_delta(std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    return {m_current_joystick_axes.at(joystick) - m_previous_joystick_axes.at(joystick)};
}

[[nodiscard]] glm::vec2 GamepadInputData::current_joystick_axes(std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    return m_current_joystick_axes.at(joystick);
}

[[nodiscard]] bool GamepadInputData::is_button_pressed(std::int32_t button, std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    return m_button_states.at(joystick).at(button);
}

void GamepadInputData::press_button(std::int32_t button, std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    m_button_states.at(joystick).at(button) = true;
    m_buttons_updated = true;
}

void GamepadInputData::release_button(std::int32_t button, std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    m_button_states.at(joystick).at(button) = false;
    m_buttons_updated = true;
}

void GamepadInputData::set_joystick_axis(std::int32_t axis, float state, std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    m_current_joystick_axes[joystick][axis] = state;
    m_joysticks_updated = true;
}

[[nodiscard]] bool GamepadInputData::was_button_pressed_once(std::int32_t button, std::int32_t joystick) {
    std::scoped_lock lock(m_input_mutex);
    if (!m_button_states.at(joystick).at(button) || !m_buttons_updated) {
        return false;
    }
    m_button_states.at(joystick).at(button) = false;
    return true;
}

} // namespace inexor::vulkan_renderer::input
