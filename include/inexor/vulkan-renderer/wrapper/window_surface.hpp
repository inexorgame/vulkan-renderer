#pragma once

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

/// @class WindowSurface
/// @brief RAII wrapper class for VkSurfaceKHR.
class WindowSurface {
    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface;

public:
    /// @brief Default constructor.
    /// @param instance [in] The Vulkan instance which will be associated with this surface.
    /// @param window [in] The window which will be associated with this surface.
    WindowSurface(const VkInstance instance, GLFWwindow *window);

    WindowSurface(const WindowSurface &) = delete;
    WindowSurface(WindowSurface &&) noexcept;

    ~WindowSurface();

    WindowSurface &operator=(const WindowSurface &) = delete;
    WindowSurface &operator=(WindowSurface &&) = default;

    [[nodiscard]] VkSurfaceKHR get() const {
        return m_surface;
    }

    const VkSurfaceKHR *operator&() const {
        return &m_surface;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
