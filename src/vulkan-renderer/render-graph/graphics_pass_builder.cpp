#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

GraphicsPassBuilder &GraphicsPassBuilder::add_color_attachment(std::weak_ptr<Texture> color_attachment,
                                                               std::optional<VkClearValue> clear_value) {
    if (color_attachment.expired()) {
        throw std::invalid_argument(
            "[GraphicsPassBuilder::add_color_attachment] Error: 'color_attachment' is an invalid pointer!");
    }
    m_color_attachments.emplace_back(std::move(color_attachment), std::move(clear_value));
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::add_depth_attachment(std::weak_ptr<Texture> depth_attachment,
                                                               std::optional<VkClearValue> clear_value) {
    if (depth_attachment.expired()) {
        throw std::invalid_argument(
            "[GraphicsPassBuilder::enable_depth_test] Error: 'depth_buffer' is an invalid pointer!");
    }
    m_depth_attachment = Attachment(std::move(depth_attachment), std::move(clear_value));
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::add_stencil_attachment(std::weak_ptr<Texture> stencil_attachment,
                                                                 std::optional<VkClearValue> clear_value) {
    if (stencil_attachment.expired()) {
        throw std::invalid_argument(
            "[GraphicsPassBuilder::add_stencil_attachment] Error: 'stencil_attachment' is an invalid pointer!");
    }
    m_stencil_attachment = Attachment(std::move(stencil_attachment), std::move(clear_value));
    return *this;
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name) {
    auto graphics_pass = std::make_shared<GraphicsPass>(std::move(name), std::move(m_on_record_cmd_buffer),
                                                        std::move(m_color_attachments), std::move(m_depth_attachment),
                                                        std::move(m_stencil_attachment));
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
    m_color_attachments = {};
    m_depth_attachment = {};
    m_stencil_attachment = {};
    m_graphics_pass_reads.clear();
}

GraphicsPassBuilder &
GraphicsPassBuilder::set_on_record(std::function<void(const CommandBuffer &)> on_record_cmd_buffer) {
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
