#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

namespace inexor::vulkan_renderer::input {

/// @brief A wrapper for keyboard and mouse input data.
/// @warning Not thread safe!
class KeyboardMouseInputData {
    std::array<std::int64_t, 2> m_previous_cursor_pos{0, 0};
    std::array<std::int64_t, 2> m_current_cursor_pos{0, 0};
    std::unordered_map<std::int32_t, bool> m_pressed_keys;
    std::unordered_map<std::int32_t, bool> m_pressed_mouse_buttons;

public:
    KeyboardMouseInputData();
    KeyboardMouseInputData(const KeyboardMouseInputData &) = delete;
    KeyboardMouseInputData(KeyboardMouseInputData &&) = delete;

    ~KeyboardMouseInputData() = default;

    KeyboardMouseInputData &operator=(const KeyboardMouseInputData &) = delete;
    KeyboardMouseInputData &operator=(KeyboardMouseInputData &&) = delete;

    /// @brief Change the key's state to pressed.
    /// @param button The key which was pressed.
    void press_key(std::int32_t key);

    /// @brief Change the key's state to unpressed.
    /// @param button The key which was released.
    void release_key(std::int32_t button);

    /// @brief Change the mouse button's state to pressed.
    /// @param button The mouse button which was pressed.
    void press_mouse_button(std::int32_t button);

    /// @brief Change the mouse button's state to unpressed.
    /// @param button The mouse button which was released.
    void release_mouse_button(std::int32_t button);

    /// @brief Set the current cursor position.
    /// @param cursor_pos_x The current x-coordinate of the cursor.
    /// @param cursor_pos_y The current y-coordinate of the cursor.
    void set_cursor_pos(double cursor_pos_x, double coursor_pos_y);

    /// @return Return a std::array of size 2 which contains the x-position in index 0 and y-position in index 1.
    [[nodiscard]] std::array<std::int64_t, 2> get_cursor_pos();

    /// @brief Check if the given mouse button is currently pressed.
    /// @param mouse_button The mouse button index.
    /// @returm True if the mouse button is pressed, false otherwise.
    [[nodiscard]] bool is_mouse_button_pressed(std::int32_t mouse_button);

    /// @brief Check if the given key is currently pressed.
    /// @param key The key index.
    /// @retur True if the key is pressed, false otherwise.
    [[nodiscard]] bool is_key_pressed(std::int32_t key);

    /// @brief Calculate the change in x- and y-position of the cursor.
    /// @return A std::array of size 2 which contains the change in x-position in index 0
    /// and the change in y-position in index 1.
    [[nodiscard]] std::array<double, 2> calculate_cursor_position_delta();
};

} // namespace inexor::vulkan_renderer::input
