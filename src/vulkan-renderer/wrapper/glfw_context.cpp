#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

GLFWContext::GLFWContext() {
    spdlog::debug("Creating GLFW context.");

    if (!glfwInit()) {
        throw std::runtime_error("Error: glfwInit failed!");
    }

    spdlog::debug("Created GLFW context successfully.");
}

GLFWContext::~GLFWContext() {
    glfwTerminate();
}

} // namespace inexor::vulkan_renderer::wrapper
