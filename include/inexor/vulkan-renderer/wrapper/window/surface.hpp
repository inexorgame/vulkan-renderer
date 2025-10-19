#pragma once

#include <GLFW/glfw3.h>
#include <volk.h>

namespace inexor::vulkan_renderer::wrapper::window {

/// @brief RAII wrapper class for VkSurfaceKHR.
class WindowSurface {
    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

public:
    /// @brief Default constructor.
    /// @param instance The Vulkan instance which will be associated with this surface.
    /// @param window The window which will be associated with this surface.
    WindowSurface(VkInstance instance, GLFWwindow *window);

    WindowSurface(const WindowSurface &) = delete;
    WindowSurface(WindowSurface &&) noexcept;

    ~WindowSurface();

    WindowSurface &operator=(const WindowSurface &) = delete;
    WindowSurface &operator=(WindowSurface &&) = delete;

    [[nodiscard]] VkSurfaceKHR get() const {
        return m_surface;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::window
