#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

void GraphicsPassBuilder::reset() {
    m_clear_value = std::nullopt;
    m_on_record = [](auto &) {};
    m_depth_test = false;
    m_buffer_reads.clear();
    m_texture_reads.clear();
    m_texture_writes.clear();
}

// TODO: Move stuff to .cpp file again. Header files should contain declarations, cpp files should contain definitions!

} // namespace inexor::vulkan_renderer::render_graph
