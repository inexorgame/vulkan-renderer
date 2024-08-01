#include "inexor/vulkan-renderer/wrapper/surface.hpp"

#include "inexor/vulkan-renderer/wrapper/vulkan_exception.hpp"

namespace inexor::vulkan_renderer::wrapper {

Surface::Surface(const VkInstance instance, GLFWwindow *window) : m_instance(instance), m_window(window) {
    // NOTE: glfwCreateWindowSurface does return a VkResult, so the VulkanException will work correctly
    if (const auto result = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface); result != VK_SUCCESS) {
        throw VulkanException("[Surface::Surface] Error: glfwCreateWindowSurface failed!", result);
    }
}

Surface::~Surface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
