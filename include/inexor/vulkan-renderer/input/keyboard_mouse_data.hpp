#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <shared_mutex>

namespace inexor::vulkan_renderer::input {

/// @brief A wrapper for keyboard and mouse input data.
class KeyboardMouseInputData {
private:
    std::array<std::int64_t, 2> m_previous_cursor_pos{0, 0};
    std::array<std::int64_t, 2> m_current_cursor_pos{0, 0};
    std::array<bool, GLFW_KEY_LAST> m_pressed_keys{false};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST> m_pressed_mouse_buttons{false};
    bool m_keyboard_updated{false};
    bool m_mouse_buttons_updated{false};
    mutable std::shared_mutex m_input_mutex;

public:
    KeyboardMouseInputData() = default;
    KeyboardMouseInputData(const KeyboardMouseInputData &) = delete;
    KeyboardMouseInputData(KeyboardMouseInputData &&) = delete;
    ~KeyboardMouseInputData() = default;

    KeyboardMouseInputData &operator=(const KeyboardMouseInputData &) = delete;
    KeyboardMouseInputData &operator=(KeyboardMouseInputData &&) = delete;

    /// @brief Change the key's state to pressed.
    /// @param key the key which was pressed
    void press_key(std::int32_t key);

    /// @brief Change the key's state to unpressed.
    /// @param key the key which was released
    /// @note key must be smaller than ``GLFW_KEY_LAST``
    void release_key(std::int32_t key);

    /// @brief Check if the given key is currently pressed.
    /// @param key the key index
    /// @note key must be smaller than ``GLFW_KEY_LAST``
    /// @return ``true`` if the key is pressed
    [[nodiscard]] bool is_key_pressed(std::int32_t key) const;

    /// @brief Checks if a key was pressed once.
    /// @param key The key index
    /// @note key must be smaller than ``GLFW_KEY_LAST``
    /// @return ``true`` if the key was pressed
    [[nodiscard]] bool was_key_pressed_once(std::int32_t key);

    /// @brief Change the mouse button's state to pressed.
    /// @param button the mouse button which was pressed
    /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST``
    void press_mouse_button(std::int32_t button);

    /// @brief Change the mouse button's state to unpressed.
    /// @param button the mouse button which was released
    /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST``
    void release_mouse_button(std::int32_t button);

    /// @brief Check if the given mouse button is currently pressed.
    /// @param button the mouse button index
    /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST``
    /// @return ``true`` if the mouse button is pressed
    [[nodiscard]] bool is_mouse_button_pressed(std::int32_t button) const;

    /// @brief Checks if a mouse button was pressed once.
    /// @param button the mouse button index
    /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST``
    /// @return ``true`` if the mouse button was pressed
    [[nodiscard]] bool was_mouse_button_pressed_once(std::int32_t button);

    /// @brief Set the current cursor position.
    /// @param pos_x the current x-coordinate of the cursor
    /// @param pos_y the current y-coordinate of the cursor
    void set_cursor_pos(double pos_x, double pos_y);

    [[nodiscard]] std::array<std::int64_t, 2> get_cursor_pos() const;

    /// @brief Calculate the change in x- and y-position of the cursor.
    /// @return a std::array of size 2 which contains the change in x-position in index 0 and the change in y-position
    /// in index 1
    [[nodiscard]] std::array<double, 2> calculate_cursor_position_delta();
};

} // namespace inexor::vulkan_renderer::input
