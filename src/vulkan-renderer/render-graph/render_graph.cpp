#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const Device &device) : m_device(device) {}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type,
                                              std::function<void()> on_update) {
    // Create a shared_ptr for the new buffer, store it, and return a weak_ptr to it.
    return m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update)));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureType type,
                                                std::optional<std::function<void()>> on_update) {
    // Create a shared_ptr for the new texture, store it, and return a weak_ptr to it.
    return m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(name), type, std::move(on_update)));
}

void RenderGraph::reset() {
    // Delete all buffers
    m_buffers.clear();
    // Delete all textures
    m_textures.clear();
}

} // namespace inexor::vulkan_renderer::render_graph
