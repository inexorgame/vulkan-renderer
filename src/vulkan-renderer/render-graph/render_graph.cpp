#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const wrapper::Device &device) : m_device(device) {}

void RenderGraph::build_graphics_pipeline() {
    // TODO: Implement
}

void RenderGraph::build_render_pass() {
    // TODO: Implement
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement
    // TODO: throw std::logic_error in case the rendergraph contains cycles!
}

void RenderGraph::create_buffers() {
    // TODO: Convert BufferUsage to VkBufferUsageFlags and VmaMemoryUsage
    // TODO: Create constant buffers and fixed size buffers
    // TODO: Create dynamic buffers
    // This should minimize memory fragmentation?
    for (const auto &buffer_request : m_buffer_resources) {
        // if constant -> create as constant
        // otherwise, do not
    }
}

void RenderGraph::create_graphics_stages() {}

void RenderGraph::create_textures() {}

void RenderGraph::compile() {
    check_for_cycles();
    determine_stage_order();
    create_buffers();
    create_textures();
    create_graphics_stages();
}

void RenderGraph::determine_stage_order() {
    // TODO: Implement dfs
}

void RenderGraph::record_command_buffer() {
    // TODO: Implement
}

void RenderGraph::render(const std::uint32_t swapchain_img_index) {
    // TODO: Implement
}

void RenderGraph::update_data() {
    // TODO: Implement
}

void RenderGraph::update_dynamic_buffers() {
    // TODO: Loop through m_dynamic_physical_buffers only...
}

} // namespace inexor::vulkan_renderer::render_graph
