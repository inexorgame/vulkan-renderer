#pragma once

#include <GLFW/glfw3.h>
#include <volk.h>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;
class Instance;
class Window;

/// RAII wrapper class for VkSurfaceKHR
class Surface {
    friend class Device;
    friend class Swapchain;

private:
    const Instance &m_instance;
    const Window &m_window;
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

public:
    /// Create a GLFW surface
    /// @param instance The Vulkan instance
    /// @param window The GLFW window to create the surface with
    Surface(const Instance &instance, const Window &window);
    Surface(const Surface &) = delete;
    // TODO: Implement me!
    Surface(Surface &&) noexcept;
    ~Surface();

    Surface &operator=(const Surface &) = delete;
    // TODO: Implement me!
    Surface &operator=(Surface &&) noexcept;
};

} // namespace inexor::vulkan_renderer::wrapper
