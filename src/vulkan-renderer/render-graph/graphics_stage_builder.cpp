#include "inexor/vulkan-renderer/render-graph/graphics_stage_builder.hpp"

namespace inexor::vulkan_renderer::render_graph {

GraphicsStageBuilder::GraphicsStageBuilder() {
    reset();
}

std::shared_ptr<GraphicsStage> GraphicsStageBuilder::build(std::string name) {
    return std::make_shared<GraphicsStage>(std::move(name), std::move(m_buffer_reads), std::move(m_texture_reads),
                                           std::move(m_texture_writes), std::move(m_on_record),
                                           std::move(m_clear_value));
}

void GraphicsStageBuilder::reset() {
    m_clear_value = std::nullopt;
    m_on_record = [](auto &) {};
    m_depth_test = false;
    m_buffer_reads.clear();
    m_texture_reads.clear();
    m_texture_writes.clear();
}

GraphicsStageBuilder &GraphicsStageBuilder::set_clear_value(const VkClearValue clear_value) {
    m_clear_value = clear_value;
    return *this;
}

GraphicsStageBuilder &GraphicsStageBuilder::set_depth_test(const bool depth_test) {
    m_depth_test = depth_test;
    return *this;
}

GraphicsStageBuilder &
GraphicsStageBuilder::set_on_record(std::function<void(const wrapper::CommandBuffer &)> on_record) {
    m_on_record = std::move(on_record);
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
