#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const Device &device) : m_device(device) {}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type,
                                              std::function<void()> on_update) {
    // Create a Buffer, store it as a shared_ptr in m_buffers, and return a weak_ptr to the newly added element
    return m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update)));
}

void RenderGraph::reset() {
    // Delete all buffers
    m_buffers.clear();
}

} // namespace inexor::vulkan_renderer::render_graph
