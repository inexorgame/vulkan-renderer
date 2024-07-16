#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

GraphicsPassBuilder::GraphicsPassBuilder(GraphicsPassBuilder &&other) noexcept {
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_write_attachments = std::move(other.m_write_attachments);
    m_graphics_pass_reads = std::move(other.m_graphics_pass_reads);
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name, const DebugLabelColor color) {
    auto graphics_pass =
        std::make_shared<GraphicsPass>(std::move(name), std::move(m_on_record_cmd_buffer),
                                       std::move(m_graphics_pass_reads), std::move(m_write_attachments), color);
    reset();
    return graphics_pass;
}

GraphicsPassBuilder &GraphicsPassBuilder::reads_from(std::weak_ptr<GraphicsPass> graphics_pass) {
    if (graphics_pass.expired()) {
        throw std::invalid_argument("[GraphicsPassBuilder::reads_from] Error: 'graphics_pass' is an invalid pointer!");
    }
    m_graphics_pass_reads.push_back(std::move(graphics_pass));
    return *this;
}

void GraphicsPassBuilder::reset() {
    m_on_record_cmd_buffer = {};
    m_write_attachments.clear();
    m_graphics_pass_reads.clear();
}

GraphicsPassBuilder &
GraphicsPassBuilder::set_on_record(std::function<void(const CommandBuffer &)> on_record_cmd_buffer) {
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::writes_to(std::weak_ptr<Texture> attachment,
                                                    std::optional<VkClearValue> clear_value) {
    if (attachment.expired()) {
        throw std::invalid_argument("[GraphicsPassBuilder::writes_to] Error: 'attachment' is an invalid pointer!");
    }
    m_write_attachments.emplace_back(std::move(attachment), std::move(clear_value));
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
