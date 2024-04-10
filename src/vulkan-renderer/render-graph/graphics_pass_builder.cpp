#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name) {
    return std::make_shared<GraphicsPass>(std::move(name), std::move(m_buffer_reads), std::move(m_texture_reads),
                                          std::move(m_texture_writes), std::move(m_on_record),
                                          std::move(m_clear_value));
}

/*
    GraphicsPass(std::string name, BufferReads buffer_reads, TextureReads texture_reads, TextureWrites texture_writes,
                 std::function<void(const wrapper::CommandBuffer &)> on_record,
                 std::optional<VkClearValue> clear_values);
*/

void GraphicsPassBuilder::reset() {
    m_clear_value = std::nullopt;
    m_on_record = [](auto &) {};
    m_depth_test = false;
    m_buffer_reads.clear();
    m_texture_reads.clear();
    m_texture_writes.clear();
}

} // namespace inexor::vulkan_renderer::render_graph
