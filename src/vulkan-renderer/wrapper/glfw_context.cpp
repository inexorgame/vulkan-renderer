#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

GLFWContext::GLFWContext() {
    spdlog::debug("Creating GLFW context.");

    m_initialized = static_cast<bool>(glfwInit());
    if (!m_initialized) {
        throw std::runtime_error("Error: glfwInit failed!");
    }

    spdlog::debug("Created GLFW context successfully.");
}

GLFWContext::GLFWContext(GLFWContext &&other) noexcept : m_initialized(other.m_initialized) {}

GLFWContext::~GLFWContext() {
    if (m_initialized) {
        glfwTerminate();
    }
}

} // namespace inexor::vulkan_renderer::wrapper
