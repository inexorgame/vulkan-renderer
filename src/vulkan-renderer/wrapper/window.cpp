#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include "inexor/vulkan-renderer/wrapper/exception.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

Window::Window(const std::string &title,
               const std::uint32_t width,
               const std::uint32_t height,
               const bool visible,
               const bool resizable,
               const Mode mode)
    : m_width(width), m_height(height), m_mode(mode) {
    if (title.empty()) {
        throw InexorException("Error: Parameter 'title' is an empty string!");
    }
    if (glfwInit() != GLFW_TRUE) {
        throw InexorException("Error: glfwInit() failed!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

    spdlog::trace("Creating window");

    GLFWmonitor *monitor = nullptr;
    if (m_mode != Mode::WINDOWED) {
        monitor = glfwGetPrimaryMonitor();
        if (m_mode == Mode::WINDOWED_FULLSCREEN) {
            const auto *video_mode = glfwGetVideoMode(monitor);
            m_width = video_mode->width;
            m_height = video_mode->height;
        }
    }

    m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), monitor, nullptr);

    if (m_window == nullptr) {
        throw InexorException("Error: glfwCreateWindow failed for window " + title + " !");
    }
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::announce_resize() {
    m_resized = true;
}

bool Window::check_resize() {
    if (m_resized) {
        m_resized = false;
        return true;
    }
    return false;
}

std::pair<std::uint32_t, std::uint32_t> Window::get_framebuffer_size() {
    int window_width = 0;
    int window_height = 0;
    glfwGetFramebufferSize(m_window, &window_width, &window_height);
    return std::make_pair(window_width, window_height);
}

void Window::poll() {
    glfwPollEvents();
}

bool Window::should_close() {
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::show() {
    glfwShowWindow(m_window);
}

void Window::set_user_ptr(void *user_ptr) {
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

void Window::set_title(const std::string &title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

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

} // namespace inexor::vulkan_renderer::wrapper
