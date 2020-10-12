#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Window {
    GLFWwindow *m_window;
    std::uint32_t m_width;
    std::uint32_t m_height;

public:
    /// @brief Creates a new window using glfw library.
    /// @param instance [in] The Vulkan instance.
    /// @param title [in] The title of the window.
    /// @param width [in] The width of the window.
    /// @param height [in] The height of the window.
    /// @param visible [in] True if the window should be visible.
    /// @param resizable [in] True if the window should be resizable.
    Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
           const bool resizable);

    Window(const Window &) = delete;
    Window(Window &&) noexcept;

    ~Window();

    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = default;

    /// @brief Waits until window size is greater than 0.
    void wait_for_focus();

    /// @brief Changes the title of the window.
    /// @param title [in] The title of the window.
    void set_title(const std::string &title);

    // @brief Stores the current Application instance in the GLFW window user pointer.
    // @note Since GLFW is a C-style API, we can't use a class method as callback for window resize.
    void set_user_ptr(void *user_ptr);

    /// @brief Sets up callback for window resize.
    /// @note Since GLFW is a C-style API, we can't use a class method as callback for window resize.
    void set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback);

    /// @brief Makes the window visible.
    void show();

    /// @brief Makes the window invisible.
    void hide();

    /// @brief Queries the current position of the cursor.
    void cursor_pos(double &pos_x, double &pos_y);

    /// @brief Checks if a button is pressed.
    /// @param button [in] The glfw button index.
    bool is_button_pressed(int button);

    /// @brief Updates window messages.
    void poll();

    /// @brief Checks if the window has received a close message.
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
