#pragma once

#include <GLFW/glfw3.h>
#include <volk.h>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Instance;

/// RAII wrapper class for VkSurfaceKHR
class Surface {

private:
    const wrapper::Instance &m_instance;
    GLFWwindow *m_window{nullptr};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

public:
    /// Create a GLFW surface
    /// @param instance The Vulkan instance
    /// @param window The GLFW window to create the surface with
    Surface(const wrapper::Instance &instance, GLFWwindow *window);
    Surface(const Surface &) = delete;
    // TODO: Implement me!
    Surface(Surface &&) noexcept;
    ~Surface();

    Surface &operator=(const Surface &) = delete;
    // TODO: Implement me!
    Surface &operator=(Surface &&) noexcept;
};

} // namespace inexor::vulkan_renderer::wrapper
