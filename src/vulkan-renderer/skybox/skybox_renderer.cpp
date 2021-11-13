#include "inexor/vulkan-renderer/skybox/skybox_renderer.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::skybox {

SkyboxRenderer::SkyboxRenderer(const wrapper::Device &device, RenderGraph *render_graph)
    : m_shader_loader(device, m_shader_files) {}

void SkyboxRenderer::draw_node(const VkCommandBuffer cmd_buf, const gltf::ModelNode *node) {
    if (node->mesh) {
        for (const auto &primitive : node->mesh->primitives) {
            vkCmdDrawIndexed(cmd_buf, primitive.index_count, 1, primitive.first_index, 0, 0);
        }
    }
    for (auto &child : node->children) {
        draw_node(cmd_buf, &child);
    }
}

void SkyboxRenderer::setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer,
                                 const TextureResource *depth_buffer, const SkyboxGpuData &skybox) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);

    auto *skybox_stage = render_graph->add<GraphicsStage>("skybox stage");

    skybox_stage->set_depth_options(true, true)
        ->uses_shaders(m_shader_loader.shaders())
        ->set_clears_screen(true)
        ->bind_buffer(skybox.vertex_buffer(), 0)
        ->bind_buffer(skybox.index_buffer(), 0)
        ->reads_from(skybox.vertex_buffer())
        ->reads_from(skybox.index_buffer())
        ->writes_to(back_buffer)
        ->writes_to(depth_buffer)
        ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
            // TODO: Implement!
        });
}

} // namespace inexor::vulkan_renderer::skybox
