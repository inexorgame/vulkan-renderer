#pragma once

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::Device;

class RenderGraph {
private:
    const Device &m_device;

public:
    /// Default constructor
    /// @param device The device wrapper
    RenderGraph(const Device &device);
};

} // namespace inexor::vulkan_renderer::render_graph
