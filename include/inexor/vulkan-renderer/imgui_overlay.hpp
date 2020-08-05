#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>

namespace inexor::vulkan_renderer::wrapper {



}

namespace inexor::vulkan_renderer {

class ImguiOverlay {
    const wrapper::Device &m_device;
    std::unique_ptr<wrapper::Shader> m_vertex_shader;
    std::unique_ptr<wrapper::Shader> m_fragment_shader;

public:
    explicit ImguiOverlay(const wrapper::Device &device);
};

} // namespace inexor::vulkan_renderer
