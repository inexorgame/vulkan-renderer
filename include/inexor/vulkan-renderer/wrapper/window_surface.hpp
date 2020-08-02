#pragma once

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class WindowSurface {
private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface;

public:
    /// Delete the copy constructor so window surfaces are move-only objects.
    WindowSurface(const WindowSurface &) = delete;
    WindowSurface(const WindowSurface &&) noexcept;

    /// Delete the copy assignment operator so windows are move-only objects.
    WindowSurface &operator=(const WindowSurface &) = delete;
    WindowSurface &operator=(WindowSurface &&other) noexcept = default;

    /// @brief Creates a new window surface.
    /// @param instance [in] The Vulkan instance.
    /// @param window [in] The glfw3 window.
    WindowSurface(const VkInstance instance, GLFWwindow *window);

    ~WindowSurface();

    [[nodiscard]] VkSurfaceKHR get() const {
        return m_surface;
    }

    const VkSurfaceKHR *operator&() const {
        return &m_surface;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
