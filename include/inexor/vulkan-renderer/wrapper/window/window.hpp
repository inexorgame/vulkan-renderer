#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <cstdint>
#include <string>

namespace inexor::vulkan_renderer::wrapper::window {

/// @brief RAII wrapper class for GLFW windows.
class Window {
public:
    enum class Mode { WINDOWED, FULLSCREEN, WINDOWED_FULLSCREEN };

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
    Mode m_mode;
    GLFWwindow *m_window{nullptr};

public:
    /// @brief Default constructor.
    /// @param title The title of the window. This will be displayed in the window bar.
    /// @param width The width of the window.
    /// @param height The height of the window.
    /// @param visible True if the window is visible after creation, false otherwise.
    /// @param resizable True if the window should be resizable, false otherwise.
    Window(const std::string &title, std::uint32_t width, std::uint32_t height, bool visible, bool resizable,
           Mode mode);

    ~Window();

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

    /// @brief Call glfwSetKeyCallback.
    /// @param key_input_callback The keyboard input callback.
    void set_keyboard_button_callback(GLFWkeyfun keyboard_button_callback);

    /// @brief Call glfwSetCursorPosCallback.
    /// @param cursor_pos_callback They cursor position callback.
    void set_cursor_position_callback(GLFWcursorposfun cursor_pos_callback);

    /// @brief Call glfwSetMouseButtonCallback.
    /// @param mouse_button_callback The mouse button callback.
    void set_mouse_button_callback(GLFWmousebuttonfun mouse_button_callback);

    /// @brief Call glfwSetScrollCallback.
    /// @param mouse_scroll_callback The mouse scroll callback.
    void set_mouse_scroll_callback(GLFWscrollfun mouse_scroll_callback);

    /// @brief Call glfwShowWindow.
    void show();

    /// @brief Call glfwPollEvents.
    static void poll();

    /// @brief Check if the window is about to close.
    /// @return ``true`` if the window will be closed.
    bool should_close();

    [[nodiscard]] auto window() const {
        return m_window;
    }

    [[nodiscard]] std::uint32_t width() const {
        return m_width;
    }

    [[nodiscard]] std::uint32_t height() const {
        return m_height;
    }

    [[nodiscard]] Mode mode() const {
        return m_mode;
    }

    [[nodiscard]] std::array<int, 2> get_framebuffer_size() const {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return {width, height};
    }
};

} // namespace inexor::vulkan_renderer::wrapper::window
