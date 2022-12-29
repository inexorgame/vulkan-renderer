#pragma once

#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>

#include <array>
#include <shared_mutex>
#include <vector>

namespace inexor::vulkan_renderer::input {
/// @brief A wrapper for gamepad input data
class GamepadInputData {
private:
    std::array<glm::vec2, 2> m_current_joystick_axes{};
    std::array<glm::vec2, 2> m_previous_joystick_axes{};
    std::array<std::array<bool, GLFW_JOYSTICK_LAST + 1>, GLFW_GAMEPAD_BUTTON_LAST> m_button_states{};
    bool m_joysticks_updated{false};
    bool m_buttons_updated{false};
    mutable std::shared_mutex m_input_mutex;

public:
    GamepadInputData() = default;

    void press_button(std::int32_t button, std::int32_t joystick = GLFW_JOYSTICK_1);

    void release_button(std::int32_t button, std::int32_t joystick = GLFW_JOYSTICK_1);

    [[nodiscard]] bool is_button_pressed(std::int32_t button, std::int32_t joystick = GLFW_JOYSTICK_1);

    [[nodiscard]] bool was_button_pressed_once(std::int32_t button, std::int32_t joystick = GLFW_JOYSTICK_1);

    void set_joystick_axis(std::int32_t axis, float state, std::int32_t joystick = GLFW_JOYSTICK_1);

    [[nodiscard]] glm::vec2 current_joystick_axes(std::int32_t joystick);

    [[nodiscard]] glm::vec2 calculate_joystick_axes_delta(std::int32_t joystick);
};
} // namespace inexor::vulkan_renderer::input
