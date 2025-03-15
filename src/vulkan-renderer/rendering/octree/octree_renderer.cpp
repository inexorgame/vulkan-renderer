#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

namespace inexor::vulkan_renderer::rendering::octree {

OctreeRenderer::OctreeRenderer(const std::weak_ptr<RenderGraph> rendergraph,
                               const std::weak_ptr<Texture> back_buffer,
                               const std::weak_ptr<Texture> depth_buffer) {
    if (rendergraph.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'rendergraph' is an invalid pointer!");
    }
    if (back_buffer.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'back_buffer' is an invalid pointer!");
    }
    if (depth_buffer.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'depth_buffer' is an invalid pointer!");
    }

    auto rg = rendergraph.lock();

    m_octree_fragment =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_octree_vertex =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");

    m_vertex_buffer = rg->add_buffer("Octree", BufferType::VERTEX_BUFFER, []() {
        // TODO: Update me!
    });

    m_index_buffer = rg->add_buffer("Octree", BufferType::INDEX_BUFFER, []() {
        // TODO: Update me!
    });

    // TODO: An API like m_octree_pipeline = rg->add_graphics_pipeline(rg->pipeline_builder().....build());
    // But do not build the pipeline, build all pipelines during rendergraph compilation?
    // Probably not a good idea, because rendergraph will be compiled every frame!!!
    // Even if there would be a compile_pipeline() step, we NEED TO KNOW THE PIPELINE LAYOUT before!
    // This means we can't create the graphics pipeline like graphics passes!

    rg->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        m_octree_pipeline = builder.add_shader(m_octree_fragment)
                                .add_shader(m_octree_fragment)
                                .set_vertex_input_attributes(std::vector<VkVertexInputAttributeDescription>{
                                    {
                                        .location = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, pos),
                                    },
                                    {
                                        .location = 1,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, color),
                                    },
                                })
                                .build("Octree");
    });

    m_octree_pass =
        rg->add_graphics_pass(rg->get_graphics_pass_builder()
                                  .set_on_record([&](const CommandBuffer &cmd_buf) {
                                      // TODO: Render multiple octrees...
                                      cmd_buf.bind_pipeline(m_octree_pipeline)
                                          .bind_vertex_buffer(m_vertex_buffer)
                                          .bind_index_buffer(m_index_buffer)
                                          .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
                                  })
                                  .writes_to(back_buffer)
                                  .writes_to(depth_buffer)
                                  .build("Octree", DebugLabelColor::RED));
}

} // namespace inexor::vulkan_renderer::rendering::octree
