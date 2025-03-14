#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <memory>

namespace inexor::vulkan_renderer::rendering::octree {

using rendering::render_graph::RenderGraph;
using wrapper::GraphicsPass;
using wrapper::Shader;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;

/// A rendering class for octrees
class OctreeRenderer {
private:
    std::shared_ptr<Shader> m_octree_vertex;
    std::shared_ptr<Shader> m_octree_fragment;
    std::weak_ptr<GraphicsPass> m_octree_pass;
    std::shared_ptr<GraphicsPipeline> m_octree_pipeline;

public:
    /// Default constructor
    /// @param rendergraph The rendergraph used to construct the octree renderer
    OctreeRenderer(std::weak_ptr<RenderGraph> rendergraph);

    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) noexcept;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) noexcept;
};

} // namespace inexor::vulkan_renderer::rendering::octree
