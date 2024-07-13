#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

namespace inexor::vulkan_renderer::render_graph {

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

void GraphicsPassBuilder::reset() {
    m_clear_value = {};
    m_on_record = [](auto &) {};
    m_enable_depth_test = false;
    m_enable_msaa = false;
    m_clear_color = false;
    m_clear_stencil = false;
}

// TODO: Move stuff to .cpp file again. Header files should contain declarations, cpp files should contain definitions!

} // namespace inexor::vulkan_renderer::render_graph
