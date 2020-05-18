#pragma once

namespace inexor::vulkan_renderer::wrapper {
class GLFWContext {
public:
    /// Delete the copy constructor so glfw contexts are move-only objects.
    GLFWContext(const GLFWContext &) = delete;
    GLFWContext(GLFWContext &&other) noexcept = default;

    /// Delete the copy assignment operator so glfw contexts are move-only objects.
    GLFWContext &operator=(const GLFWContext &) = delete;
    GLFWContext &operator=(GLFWContext &&) noexcept = default;

    GLFWContext();

    ~GLFWContext();
};

} // namespace inexor::vulkan_renderer::wrapper
