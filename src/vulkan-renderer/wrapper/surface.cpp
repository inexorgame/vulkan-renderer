#include "inexor/vulkan-renderer/wrapper/surface.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

namespace inexor::vulkan_renderer::wrapper {

Surface::Surface(const VkInstance instance, GLFWwindow *window) : m_instance(instance), m_window(window) {
    // NOTE: glfwCreateWindowSurface does indeed return a VkResult
    if (const auto result = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface); result != VK_SUCCESS) {
        throw VulkanException("[Surface::Surface] Error: glfwCreateWindowSurface failed!", result);
    }
}

Surface::~Surface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
