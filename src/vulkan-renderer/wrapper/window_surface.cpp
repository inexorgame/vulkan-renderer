#include "inexor/vulkan-renderer/wrapper/window_surface.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

WindowSurface::WindowSurface(const VkInstance instance, GLFWwindow *window) : m_instance(instance) {
    assert(instance);
    assert(window);

    spdlog::debug("Creating window surface.");

    if (const auto result = glfwCreateWindowSurface(instance, window, nullptr, &m_surface); result != VK_SUCCESS) {
        throw VulkanException("Error: glfwCreateWindowSurface failed!", result);
    }

    spdlog::debug("Created window surface successfully");
}

WindowSurface::WindowSurface(WindowSurface &&other) noexcept {
    m_instance = other.m_instance;
    m_surface = std::exchange(other.m_surface, nullptr);
}

WindowSurface::~WindowSurface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
