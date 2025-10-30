#include "inexor/vulkan-renderer/wrapper/window/surface.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::window {

using tools::VulkanException;

WindowSurface::WindowSurface(const VkInstance instance, GLFWwindow *window) : m_instance(instance) {
    assert(instance);
    assert(window);

    spdlog::trace("Creating window surface");

    if (const auto result = glfwCreateWindowSurface(instance, window, nullptr, &m_surface); result != VK_SUCCESS) {
        throw VulkanException("Error: glfwCreateWindowSurface failed!", result);
    }
}

WindowSurface::WindowSurface(WindowSurface &&other) noexcept {
    m_instance = other.m_instance;
    m_surface = std::exchange(other.m_surface, nullptr);
}

WindowSurface::~WindowSurface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::window
