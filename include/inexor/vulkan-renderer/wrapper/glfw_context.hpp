#pragma once

namespace inexor::vulkan_renderer::wrapper {

class GLFWContext {
    bool m_initialized{false};

public:
    GLFWContext();
    GLFWContext(const GLFWContext &) = delete;
    GLFWContext(GLFWContext &&) noexcept;
    ~GLFWContext();

    GLFWContext &operator=(const GLFWContext &) = delete;
    GLFWContext &operator=(GLFWContext &&) = default;
};

} // namespace inexor::vulkan_renderer::wrapper
