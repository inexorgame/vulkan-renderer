#pragma once

#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/world/octree_gpu_data.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

template <typename VertexType, typename IndexType = std::uint32_t>
class OctreeRenderer {
public:
    OctreeRenderer() = default;
    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) = delete;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) = delete;

    /// Render a given cube or octree.
    /// @param render_graph The rendergraph
    /// @param back_buffer The back buffer to render to
    /// @param depth_buffer The depth buffer to render to
    /// @param shaders The shaders which are used for rendering
    /// @param octree_data The octree gpu data for rendering
    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const std::vector<wrapper::Shader> &shaders,
                     const world::OctreeGPUData<VertexType, IndexType, UniformBufferObject> &octree_data) {

        assert(render_graph);
        assert(back_buffer);
        assert(depth_buffer);
        assert(!shaders.empty());

        // TODO: Render multiple octrees in ONE stage!
        auto *octree_stage = render_graph->add<GraphicsStage>("octree stage");

        octree_stage->set_depth_options(true, true)
            ->bind_buffer(octree_data.vertex_buffer(), 0)
            ->bind_buffer(octree_data.index_buffer(), 0)
            ->uses_shaders(shaders)
            ->set_clears_screen(true)
            ->writes_to(back_buffer)
            ->writes_to(depth_buffer)
            ->reads_from(octree_data.vertex_buffer())
            ->reads_from(octree_data.index_buffer())
            ->add_descriptor_layout(octree_data.descriptor_set_layout())
            ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
                cmd_buf.bind_descriptor(octree_data.descriptor_set(), physical.pipeline_layout());
                cmd_buf.draw_indexed(octree_data.index_count());
            });
    }
};

} // namespace inexor::vulkan_renderer::world
