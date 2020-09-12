#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @class Window
/// @brief RAII wrapper class for GLFW windows.
class Window {
    GLFWwindow *m_window;
    std::uint32_t m_width;
    std::uint32_t m_height;

public:
    /// @brief Default constructor.
    /// @param title [in] The title of the window. This will be displayed in the window bar.
    /// @param width [in] The width of the window.
    /// @param height [in] The height of the window.
    /// @param visible [in] True if the window is visible after creation, false otherwise.
    /// @param resizable [in] True if the window should be resizable, false otherwise.
    Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
           const bool resizable);

    Window(const Window &) = delete;
    Window(Window &&) noexcept;

    ~Window();

    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = default;

    /// @brief In case the window has been minimized, process events until it has been restored.
    void wait_for_focus();

    /// @brief Changes the window title.
    /// @param title [in] The new title of the window.
    void set_title(const std::string &title);

    /// @brief Sets the GLFW window user pointer.
    /// @param user_ptr [in] The window user pointer.
    void set_user_ptr(void *user_ptr);

    /// @brief Sets up the window resize callback.
    /// @param frame_buffer_resize_callback [in] The window resize callback.
    void set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback);

    /// @brief Calls glfwShowWindow.
    void show();

    /// @brief Calls glfwHideWindow.
    void hide();

    /// @brief Sets the cursor position.
    /// @param pos_x [in] The x cursor position.
    /// @param pos_y [in] The y cursor position.
    void cursor_pos(double &pos_x, double &pos_y);

    /// @brief Checks if a specifiy button is pressed.
    /// @param button [in] The button to check.
    /// @return True if the button is pressed, false otherwise.
    /// @todo: Use a callback instead!
    bool is_button_pressed(int button);

    /// @brief Calld glfwPollEvents.
    void poll();

    /// @brief Checks if the window is about to close.
    /// @return True if the window will be closed, false otherwise.
    bool should_close();

    [[nodiscard]] GLFWwindow *get() const {
        return m_window;
    }

    [[nodiscard]] int width() const {
        return m_width;
    }

    [[nodiscard]] int height() const {
        return m_height;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
