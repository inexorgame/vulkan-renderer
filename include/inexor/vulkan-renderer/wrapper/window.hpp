#pragma once

#include <GLFW/glfw3.h>

#include <volk.h>

#include <cstdint>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// RAII wrapper class for GLFW windows and VkSurfaceKHR
class Window {
    friend class Surface;

public:
    enum class Mode { WINDOWED, FULLSCREEN, WINDOWED_FULLSCREEN };

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
    Mode m_mode;
    GLFWwindow *m_window{nullptr};

public:
    /// Default constructor
    /// @param title The title of the window. This will be displayed in the window bar.
    /// @param width The width of the window.
    /// @param height The height of the window.
    /// @param visible True if the window is visible after creation, false otherwise.
    /// @param resizable True if the window should be resizable, false otherwise.
    Window(const std::string &title,
           std::uint32_t width,
           std::uint32_t height,
           bool visible,
           bool resizable,
           Mode mode);

    Window(const Window &) = delete;
    Window(Window &&) = delete;
    ~Window();

    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = delete;

    /// Get the framebuffer size
    /// @param width [out] The width of the framebuffer
    /// @param height [out] The height of the framebuffer
    void get_framebuffer_size(int *width, int *height);

    /// Change the window title
    /// @param title The new title of the window.
    void set_title(const std::string &title);

    /// Set the GLFW window user pointer
    /// @param user_ptr The window user pointer.
    // @note Since GLFW is a C-style API, we can't use a class method as callback for window resize.
    void set_user_ptr(void *user_ptr);

    /// Set up the window resize callback
    /// @param frame_buffer_resize_callback The window resize callback.
    void set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback);

    /// Call glfwSetKeyCallback
    /// @param key_input_callback The keyboard input callback.
    void set_keyboard_button_callback(GLFWkeyfun keyboard_button_callback);

    /// Call glfwSetCursorPosCallback
    /// @param cursor_pos_callback They cursor position callback.
    void set_cursor_position_callback(GLFWcursorposfun cursor_pos_callback);

    /// Call glfwSetMouseButtonCallback
    /// @param mouse_button_callback The mouse button callback.
    void set_mouse_button_callback(GLFWmousebuttonfun mouse_button_callback);

    /// Call glfwSetScrollCallback
    /// @param mouse_scroll_callback The mouse scroll callback.
    void set_mouse_scroll_callback(GLFWscrollfun mouse_scroll_callback);

    /// Call glfwShowWindow
    void show();

    /// Call glfwPollEvents
    static void poll();

    /// Check if the window is about to close.
    /// @return ``true`` if the window will be closed.
    bool should_close();

    /// In case the window has been minimized, process events until it has been restored.
    void wait_for_focus();

    [[nodiscard]] std::uint32_t width() const {
        return m_width;
    }

    [[nodiscard]] std::uint32_t height() const {
        return m_height;
    }

    [[nodiscard]] Mode mode() const {
        return m_mode;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
