#include "inexor/vulkan-renderer/skybox/skybox_renderer.hpp"

#include "inexor/vulkan-renderer/gltf/vertex.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::skybox {

SkyboxRenderer::SkyboxRenderer(RenderGraph *render_graph)
    : m_shader_loader(render_graph->device_wrapper(), m_shader_files, "skybox") {}

void SkyboxRenderer::draw_node(const wrapper::CommandBuffer &cmd_buf, const gltf::ModelNode &node) {
    if (node.mesh) {
        for (const auto &primitive : node.mesh->primitives) {
            cmd_buf.draw_indexed(primitive.index_count, primitive.first_index);
        }
    }
    for (const auto &child : node.children) {
        draw_node(cmd_buf, *child);
    }
}

void SkyboxRenderer::setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer,
                                 const TextureResource *depth_buffer, const SkyboxGpuData &skybox) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);

    auto skybox_stage = render_graph->add<GraphicsStage>("skybox")
                            ->set_depth_options(false, false)
                            ->uses_shaders(m_shader_loader.shaders())
                            ->set_clears_screen(true) // TODO: yeetari told me we don't need to do this anymore now
                            ->set_cull_mode(VK_CULL_MODE_FRONT_BIT)
                            ->bind_buffer(skybox.vertex_buffer(), 0) // TODO: Unify bind with reads_from?
                            ->bind_buffer(skybox.index_buffer(), 0)
                            ->reads_from(skybox.vertex_buffer())
                            ->reads_from(skybox.index_buffer())
                            ->writes_to(back_buffer)
                            ->writes_to(depth_buffer)
                            ->add_descriptor_set_layout(skybox.descriptor_set_layout())
                            ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
                                cmd_buf.bind_descriptor_set(skybox.descriptor_set(), physical.pipeline_layout());
                                for (const auto &node : skybox.nodes()) {
                                    draw_node(cmd_buf, *node);
                                }
                            });
}

} // namespace inexor::vulkan_renderer::skybox
