#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Window::Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
               const bool resizable, const Mode mode)
    : m_width(width), m_height(height), m_mode(mode) {
    assert(!title.empty());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

    spdlog::debug("Creating window.");

    GLFWmonitor *monitor = nullptr;
    if (m_mode != Mode::WINDOWED) {
        monitor = glfwGetPrimaryMonitor();
        if (m_mode == Mode::WINDOWED_FULLSCREEN) {
            const auto *mode = glfwGetVideoMode(monitor);
            m_width = mode->width;
            m_height = mode->height;
        }
    }

    m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), monitor, nullptr);

    if (m_window == nullptr) {
        throw std::runtime_error("Error: glfwCreateWindow failed for window " + title + " !");
    }
}

Window::Window(Window &&other) noexcept
    : m_window(std::exchange(other.m_window, nullptr)), m_width(other.m_width), m_height(other.m_height),
      m_mode(other.m_mode) {}

void Window::wait_for_focus() {
    int current_width = 0;
    int current_height = 0;

    do {
        glfwWaitEvents();
        glfwGetFramebufferSize(m_window, &current_width, &current_height);
    } while (current_width == 0 || current_height == 0);

    m_width = current_width;
    m_height = current_height;
}

void Window::set_title(const std::string &title) {
    assert(!title.empty());
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::set_user_ptr(void *user_ptr) {
    assert(user_ptr);
    glfwSetWindowUserPointer(m_window, user_ptr);
}

void Window::set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback) {
    glfwSetFramebufferSizeCallback(m_window, frame_buffer_resize_callback);
}

void Window::set_keyboard_button_callback(GLFWkeyfun keyboard_button_callback) {
    glfwSetKeyCallback(m_window, keyboard_button_callback);
}

void Window::set_cursor_position_callback(GLFWcursorposfun cursor_pos_callback) {
    glfwSetCursorPosCallback(m_window, cursor_pos_callback);
}

void Window::set_mouse_button_callback(GLFWmousebuttonfun mouse_button_callback) {
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
}

void Window::set_mouse_scroll_callback(GLFWscrollfun mouse_scroll_callback) {
    glfwSetScrollCallback(m_window, mouse_scroll_callback);
}

void Window::show() {
    glfwShowWindow(m_window);
}

void Window::poll() {
    glfwPollEvents();
}

bool Window::should_close() {
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}

} // namespace inexor::vulkan_renderer::wrapper
