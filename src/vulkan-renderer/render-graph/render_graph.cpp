#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const Device &device) : m_device(device) {}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type,
                                              std::function<void()> on_update) {
    return m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update)));
}

std::weak_ptr<GraphicsPass> RenderGraph::add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass) {
    return m_graphics_passes.emplace_back(std::move(graphics_pass));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureUsage usage, const VkFormat format,
                                                const std::uint32_t width, const std::uint32_t height,
                                                const std::uint32_t channels, const VkSampleCountFlagBits sample_count,
                                                std::function<void()> on_update) {
    return m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(name), usage, format, width, height,
                                                             channels, sample_count, std::move(on_update)));
}

void RenderGraph::reset() {
    m_buffers.clear();
    m_textures.clear();
    m_graphics_passes.clear();
}

} // namespace inexor::vulkan_renderer::render_graph
