#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @brief RAII wrapper class for GLFW windows.
class Window {
    GLFWwindow *m_window;
    std::uint32_t m_width;
    std::uint32_t m_height;

public:
    /// @brief Default constructor.
    /// @param title The title of the window. This will be displayed in the window bar.
    /// @param width The width of the window.
    /// @param height The height of the window.
    /// @param visible True if the window is visible after creation, false otherwise.
    /// @param resizable True if the window should be resizable, false otherwise.
    Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
           const bool resizable);

    Window(const Window &) = delete;
    Window(Window &&) noexcept;

    ~Window();

    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = default;

    /// @brief In case the window has been minimized, process events until it has been restored.
    void wait_for_focus();

    /// @brief Change the window title.
    /// @param title The new title of the window.
    void set_title(const std::string &title);

    /// @brief Set the GLFW window user pointer.
    /// @param user_ptr The window user pointer.
    // @note Since GLFW is a C-style API, we can't use a class method as callback for window resize.
    void set_user_ptr(void *user_ptr);

    /// @brief Set up the window resize callback.
    /// @param frame_buffer_resize_callback The window resize callback.
    void set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback);

    /// @brief Call glfwShowWindow.
    void show();

    /// @brief Call glfwHideWindow.
    void hide();

    /// @brief Set the cursor position.
    /// @param pos_x The x cursor position.
    /// @param pos_y The y cursor position.
    void cursor_pos(double &pos_x, double &pos_y);

    /// @brief Check if a specifiy button is pressed.
    /// @param button The button to check.
    /// @return ``true`` if the button is pressed.
    /// @todo: Use a callback instead!
    bool is_button_pressed(int button);

    /// @brief Call glfwPollEvents.
    void poll();

    /// @brief Check if the window is about to close.
    /// @return ``true`` if the window will be closed.
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
