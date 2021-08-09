#include "inexor/vulkan-renderer/gltf/model_renderer.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

void ModelRenderer::render_model_node(const ModelGpuData &model, const wrapper::CommandBuffer &cmd_buf,
                                      const VkPipelineLayout layout, const ModelNode &node) {
    if (!node.mesh.empty()) {
        // TODO: Render!
    }

    for (const auto &child_node : node.children) {
        render_model_node(model, cmd_buf, layout, child_node);
    }
}

void ModelRenderer::render_model_nodes(const ModelGpuData &model, const wrapper::CommandBuffer &cmd_buf,
                                       const VkPipelineLayout layout) {
    for (const auto &node : model.nodes()) {
        render_model_node(model, cmd_buf, layout, node);
    }
}

void ModelRenderer::setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer,
                                const TextureResource *depth_buffer, const std::vector<wrapper::Shader> &shaders,
                                const ModelGpuData &model) {

    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);

    // TODO: Can we turn this into one builder pattern call?
    auto *gltf_stage = render_graph->add<GraphicsStage>("gltf stage");

    gltf_stage->uses_shaders(shaders)
        ->bind_buffer(model.vertex_buffer(), 0)
        ->set_depth_options(true, true)
        ->writes_to(back_buffer)
        ->writes_to(depth_buffer)
        ->reads_from(model.index_buffer())
        ->reads_from(model.vertex_buffer())
        ->add_descriptor_layout(model.descriptor_layout());

    gltf_stage->add_push_constant_range(sizeof(glm::mat4));

    gltf_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(*model.descriptor(), 0, physical.pipeline_layout());
        render_model_nodes(model, cmd_buf, physical.pipeline_layout());
    });
}

} // namespace inexor::vulkan_renderer::gltf
