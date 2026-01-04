#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::Device;

class RenderGraph {
private:
    // The device wrapper
    const Device &m_device;

    /// The buffers (vertex buffers, index buffers, uniform buffers...)
    std::vector<std::shared_ptr<render_graph::Buffer>> m_buffers;

public:
    /// Default constructor
    /// @param device The device wrapper
    RenderGraph(const Device &device);

    /// Add a buffer to the rendergraph
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The update function of the buffer
    /// @return A weak pointer to the buffer resource
    std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type, std::function<void()> on_update);

    /// Reset the entire rendergraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
