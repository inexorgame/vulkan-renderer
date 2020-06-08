#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Window::Window(Window &&other) noexcept : window(std::exchange(other.window, nullptr)) {}

Window::Window(const std::string &title, const std::uint32_t width, const std::uint32_t height, const bool visible,
               const bool resizable)
    : width(width), height(height) {
    assert(!title.empty());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, visible);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    spdlog::debug("Creating window.");

    this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        throw std::runtime_error("Error: glfwCreateWindow failed for window " + title + " !");
    }
}

void Window::wait_for_focus() {
    assert(window);

    int current_width = 0;
    int current_height = 0;

    while (current_width == 0 || current_height == 0) {
        glfwGetFramebufferSize(window, &current_width, &current_height);
        glfwWaitEvents();
    }

    width = current_width;
    height = current_height;
}

void Window::set_title(const std::string &title) {
    assert(window);
    assert(!title.empty());
    glfwSetWindowTitle(window, title.c_str());
}

void Window::set_user_ptr(void *user_ptr) {
    assert(window);
    assert(user_ptr);
    glfwSetWindowUserPointer(window, user_ptr);
}

void Window::set_resize_callback(GLFWframebuffersizefun frame_buffer_resize_callback) {
    assert(window);
    assert(frame_buffer_resize_callback);
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);
}

void Window::show() {
    assert(window);
    glfwShowWindow(window);
}

void Window::hide() {
    assert(window);
    glfwHideWindow(window);
}

void Window::get_cursor_pos(double &pos_x, double &pos_y) {
    assert(window);
    glfwGetCursorPos(window, &pos_x, &pos_y);
}

bool Window::is_button_pressed(int button) {
    assert(window);
    return glfwGetMouseButton(window, button);
}

void Window::poll() {
    assert(window);
    glfwPollEvents();
}

bool Window::should_close() {
    assert(window);
    return glfwWindowShouldClose(window);
}

Window::~Window() {
    spdlog::trace("Destroying window.");
    glfwDestroyWindow(window);
}

} // namespace inexor::vulkan_renderer::wrapper
