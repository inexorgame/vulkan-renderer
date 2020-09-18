#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Window::Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
               const bool resizable)
    : m_width(width), m_height(height) {
    assert(!title.empty());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, visible);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    spdlog::debug("Creating window.");

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!m_window) {
        throw std::runtime_error("Error: glfwCreateWindow failed for window " + title + " !");
    }
}

Window::Window(Window &&other) noexcept : m_window(std::exchange(other.m_window, nullptr)) {}

void Window::wait_for_focus() {
    assert(m_window);

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
    assert(m_window);
    assert(!title.empty());
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::set_user_ptr(void *user_ptr) {
    assert(m_window);
    assert(user_ptr);
    glfwSetWindowUserPointer(m_window, user_ptr);
}

void Window::set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback) {
    assert(m_window);
    assert(frame_buffer_resize_callback);
    glfwSetFramebufferSizeCallback(m_window, frame_buffer_resize_callback);
}

void Window::show() {
    assert(m_window);
    glfwShowWindow(m_window);
}

void Window::hide() {
    assert(m_window);
    glfwHideWindow(m_window);
}

std::array<double, 2> Window::cursor_pos() {
    assert(m_window);
    double cursor_pos_x{0.0};
    double cursor_pos_y{0.0};

    glfwGetCursorPos(m_window, &cursor_pos_x, &cursor_pos_y);

    std::array<double, 2> cursor_pos;
    cursor_pos[0] = cursor_pos_x;
    cursor_pos[1] = cursor_pos_y;
    return cursor_pos;
}

bool Window::is_button_pressed(int button) {
    assert(m_window);
    return glfwGetMouseButton(m_window, button);
}

void Window::poll() {
    assert(m_window);
    glfwPollEvents();
}

bool Window::should_close() {
    assert(m_window);
    return glfwWindowShouldClose(m_window);
}

Window::~Window() {
    if (m_window != nullptr) {
        spdlog::trace("Destroying window.");
        glfwDestroyWindow(m_window);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
