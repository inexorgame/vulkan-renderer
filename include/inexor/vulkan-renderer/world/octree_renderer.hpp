#pragma once

#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/octree_gpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

template <typename VertexType, typename IndexType = std::uint32_t>
class OctreeRenderer {
private:
    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/octree/octree.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "octree vertex shader"},
        {"shaders/octree/octree.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "octree fragment shader"}};

    wrapper::ShaderLoader m_shader_loader;

public:
    OctreeRenderer(const wrapper::Device &device) : m_shader_loader(device, m_shader_files, "octree") {}

    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) = delete;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) = delete;

    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const OctreeGpuData<DefaultUBO, VertexType, IndexType> &octree_data) {
        assert(render_graph);
        assert(back_buffer);
        assert(depth_buffer);

        auto octree_stage =
            render_graph->add<GraphicsStage>("octree")
                ->set_depth_options(true, true)
                ->bind_buffer(octree_data.vertex_buffer(), 0)
                ->bind_buffer(octree_data.index_buffer(), 0)
                ->uses_shaders(m_shader_loader.shaders())
                ->set_clears_screen(false)
                ->writes_to(back_buffer)
                ->writes_to(depth_buffer)
                ->reads_from(octree_data.vertex_buffer())
                ->reads_from(octree_data.index_buffer())
                ->add_descriptor_set_layout(octree_data.descriptor_set_layout())
                ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
                    cmd_buf.bind_descriptor_set(octree_data.descriptor_set(), physical.pipeline_layout());
                    cmd_buf.draw_indexed(octree_data.index_count());
                });
    }
};

} // namespace inexor::vulkan_renderer::world
