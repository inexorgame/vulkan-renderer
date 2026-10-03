#include "inexor/vulkan-renderer/render-graph/render_module.hpp"

#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

RenderModule::RenderModule(std::shared_ptr<RenderGraph> render_graph, std::string name)
    : m_render_graph(std::move(render_graph)), m_name(std::move(name)) {
    if (!m_render_graph) {
        throw tools::InexorException("Error: Parameter 'render_graph' is invalid!");
    }
}

std::weak_ptr<GraphicsPass> RenderModule::add_graphics_pass(OnBuildGraphicsPass on_build_graphics_pass) {
    return m_render_graph->add_graphics_pass(std::move(on_build_graphics_pass));
}

void RenderModule::add_graphics_pipeline(OnBuildGraphicsPipeline on_build_graphics_pipeline) {
    m_render_graph->add_graphics_pipeline(std::move(on_build_graphics_pipeline));
}

} // namespace inexor::vulkan_renderer::render_graph
