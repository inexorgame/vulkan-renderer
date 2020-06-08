#include "inexor/vulkan-renderer/wrapper/window_surface.hpp"

namespace inexor::vulkan_renderer::wrapper {

WindowSurface::WindowSurface(const VkInstance instance, GLFWwindow *window) : instance(instance) {
    assert(instance);
    assert(window);

    spdlog::debug("Creating window surface.");

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Error: glfwCreateWindowSurface failed!");
    }

    spdlog::debug("Created window surface successfully");
}

WindowSurface::~WindowSurface() {
    spdlog::trace("Destroying window surface.");
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
