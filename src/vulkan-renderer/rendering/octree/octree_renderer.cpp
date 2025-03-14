#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

namespace inexor::vulkan_renderer::rendering::octree {

using wrapper::DebugLabelColor;
using wrapper::commands::CommandBuffer;

OctreeRenderer::OctreeRenderer(const std::weak_ptr<RenderGraph> rendergraph) {
    auto rg = rendergraph.lock();

    m_octree_fragment =
        std::make_shared<Shader>(rg->device(), "octree", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_octree_vertex =
        std::make_shared<Shader>(rg->device(), "octree", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");

    rg->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        //
        m_octree_pipeline = builder.add_shader(m_octree_fragment).add_shader(m_octree_fragment).build("Octree");
    });

    m_octree_pass = rg->add_graphics_pass(rg->get_graphics_pass_builder()
                                              .set_on_record([&](const CommandBuffer &cmd_buf) {
                                                  // TODO
                                                  cmd_buf.bind_pipeline(m_octree_pipeline);
                                              })
                                              .build("Octree", DebugLabelColor::RED));
}

} // namespace inexor::vulkan_renderer::rendering::octree
