#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

GraphicsPassBuilder::GraphicsPassBuilder(GraphicsPassBuilder &&other) noexcept {
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_write_attachments = std::move(other.m_write_attachments);
    m_write_swapchains = std::move(other.m_write_swapchains);
    m_graphics_pass_reads = std::move(other.m_graphics_pass_reads);
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name, const DebugLabelColor pass_debug_color) {
    auto graphics_pass = std::make_shared<GraphicsPass>(
        std::move(name), std::move(m_on_record_cmd_buffer), std::move(m_graphics_pass_reads),
        std::move(m_write_attachments), std::move(m_write_swapchains), pass_debug_color);
    reset();
    return graphics_pass;
}

GraphicsPassBuilder &GraphicsPassBuilder::conditionally_reads_from(std::weak_ptr<GraphicsPass> graphics_pass,
                                                                   const bool condition) {
    if (!graphics_pass.expired() && condition) {
        m_graphics_pass_reads.push_back(std::move(graphics_pass));
    }
    // NOTE: No exception is thrown if this graphics pass is expired because it's an optional pass!
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::reads_from(const std::weak_ptr<GraphicsPass> graphics_pass) {
    if (graphics_pass.expired()) {
        throw std::invalid_argument("[GraphicsPassBuilder::reads_from] Error: 'graphics_pass' is an invalid pointer!");
    }
    m_graphics_pass_reads.push_back(std::move(graphics_pass));
    return *this;
}

void GraphicsPassBuilder::reset() {
    m_on_record_cmd_buffer = {};
    m_graphics_pass_reads.clear();
    m_write_attachments.clear();
}

GraphicsPassBuilder &GraphicsPassBuilder::set_on_record(OnRecordCommandBufferForPass on_record_cmd_buffer) {
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::writes_to(const std::weak_ptr<Texture> attachment,
                                                    const std::optional<VkClearValue> clear_value) {
    if (attachment.expired()) {
        throw std::invalid_argument("[GraphicsPassBuilder::writes_to] Error: 'attachment' is an invalid pointer!");
    }
    m_write_attachments.emplace_back(std::make_pair(std::move(attachment), std::move(clear_value)));
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::writes_to(const std::weak_ptr<Swapchain> swapchain,
                                                    const std::optional<VkClearValue> clear_value) {
    if (swapchain.expired()) {
        throw std::invalid_argument("[GraphicsPassBuilder::writes_to] Error: 'swapchain' is an invalid pointer!");
    }
    m_write_swapchains.emplace_back(std::make_pair(std::move(swapchain), std::move(clear_value)));
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
