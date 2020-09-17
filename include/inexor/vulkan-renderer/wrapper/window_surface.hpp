#pragma once

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class WindowSurface {
    VkInstance m_instance;
    VkSurfaceKHR m_surface;

public:
    /// @brief Creates a new window surface.
    /// @param instance [in] The Vulkan instance.
    /// @param window [in] The glfw3 window.
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
