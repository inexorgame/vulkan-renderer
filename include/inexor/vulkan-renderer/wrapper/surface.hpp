#pragma once

#include <GLFW/glfw3.h>
#include <volk.h>

namespace inexor::vulkan_renderer::wrapper {

/// RAII wrapper class for VkSurfaceKHR
class Surface {

private:
    const VkInstance m_instance{VK_NULL_HANDLE};
    GLFWwindow *m_window{nullptr};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

public:
    /// Create a GLFW surface
    /// @param instance The Vulkan instance
    /// @param window The GLFW window to create the surface with
    Surface(VkInstance instance, GLFWwindow *window);
    Surface(const Surface &) = delete;
    // TODO: Implement me!
    Surface(Surface &&) noexcept;
    ~Surface();

    Surface &operator=(const Surface &) = delete;
    // TODO: Implement me!
    Surface &operator=(Surface &&) noexcept;

    [[nodiscard]] VkSurfaceKHR surface() const {
        return m_surface;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
