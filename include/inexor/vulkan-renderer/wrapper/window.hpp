#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Window {
private:
    GLFWwindow *window;
    std::uint32_t width;
    std::uint32_t height;

public:
    /// Delete the copy constructor so windows are move-only objects.
    Window(const Window &) = delete;
    Window(Window &&other) noexcept;

    /// Delete the copy assignment operator so windows are move-only objects.
    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) noexcept = default;

    /// @brief Creates a new window using glfw library.
    /// @param instance [in] The Vulkan instance.
    /// @param title [in] The title of the window.
    /// @param width [in] The width of the window.
    /// @param height [in] The height of the window.
    /// @param visible [in] True if the window should be visible.
    /// @param resizable [in] True if the window should be resizable.
    Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
           const bool resizable);

    ~Window();

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
    void get_cursor_pos(double &pos_x, double &pos_y);

    /// @brief Checks if a button is pressed.
    /// @param button [in] The glfw button index.
    bool is_button_pressed(int button);

    /// @brief Updates window messages.
    void poll();

    /// @brief Checks if the window has received a close message.
    bool should_close();

    [[nodiscard]] GLFWwindow *get() const {
        return window;
    }

    [[nodiscard]] int get_width() const {
        return width;
    }

    [[nodiscard]] int get_height() const {
        return height;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
