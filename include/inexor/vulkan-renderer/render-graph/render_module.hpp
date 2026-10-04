#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"

namespace inexor::vulkan_renderer::wrapper::pipelines {

class GraphicsPipeline;

} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {

// Forward declarations

class GraphicsPass;
class RenderGraph;

class RenderModule {

private:
    std::shared_ptr<RenderGraph> m_render_graph;
    std::string m_name;
    std::vector<std::weak_ptr<GraphicsPass>> m_graphics_passes;
    std::vector<std::weak_ptr<::inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline>> m_graphics_pipelines;

public:
    using OnBuildGraphicsPass = std::function<std::shared_ptr<GraphicsPass>(GraphicsPassBuilder &)>;
    using OnBuildGraphicsPipeline =
        std::function<std::shared_ptr<::inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline>(
            ::inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder &)>;

    RenderModule(std::shared_ptr<RenderGraph> render_graph, std::string name);

    [[nodiscard]] std::weak_ptr<GraphicsPass> add_graphics_pass(OnBuildGraphicsPass on_build_graphics_pass);

    void add_graphics_pipeline(OnBuildGraphicsPipeline on_build_graphics_pipeline);
};

} // namespace inexor::vulkan_renderer::render_graph
