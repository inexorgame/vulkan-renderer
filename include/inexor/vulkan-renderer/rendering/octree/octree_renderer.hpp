#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/vec3.hpp>

#include <memory>

namespace inexor::vulkan_renderer::rendering::octree {

using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::RenderGraph;
using render_graph::Texture;
using wrapper::DebugLabelColor;
using wrapper::GraphicsPass;
using wrapper::Shader;
using wrapper::commands::CommandBuffer;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;

struct OctreeVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

/// A rendering class for octrees
class OctreeRenderer {
private:
    std::shared_ptr<Shader> m_octree_vertex;
    std::shared_ptr<Shader> m_octree_fragment;
    std::weak_ptr<GraphicsPass> m_octree_pass;
    std::shared_ptr<GraphicsPipeline> m_octree_pipeline;

    // TODO: Render multiple octrees...
    std::weak_ptr<Buffer> m_vertex_buffer;
    std::weak_ptr<Buffer> m_index_buffer;

    // TODO: Render multiple octrees...
    std::vector<OctreeVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;

public:
    /// Default constructor
    /// @param rendergraph The rendergraph used to build octree renderer on
    /// @param back_buffer The back buffer to render to
    /// @param depth_buffer The depth buffer to render to
    OctreeRenderer(std::weak_ptr<RenderGraph> rendergraph,
                   std::weak_ptr<Texture> back_buffer,
                   std::weak_ptr<Texture> depth_buffer);

    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) noexcept;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) noexcept;

    [[nodiscard]] const std::weak_ptr<GraphicsPass> get_pass() const {
        return m_octree_pass;
    }

    // TODO: Add octree world
    // Since rendergraph is compiled every frame anyways, calling add_buffer here should be no problem
    // void add_octree();
};

} // namespace inexor::vulkan_renderer::rendering::octree
