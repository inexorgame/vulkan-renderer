#pragma once

namespace inexor::vulkan_renderer::wrapper {

class GLFWContext {
public:
    GLFWContext();
    GLFWContext(const GLFWContext &) = delete;
    GLFWContext(GLFWContext &&) noexcept = default;
    ~GLFWContext();

    GLFWContext &operator=(const GLFWContext &) = delete;
    GLFWContext &operator=(GLFWContext &&) = default;
};

} // namespace inexor::vulkan_renderer::wrapper
