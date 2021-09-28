#include "inexor/vulkan-renderer/gltf/gltf_pbr_renderer.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

void ModelRenderer::draw_node(const ModelGpuData &model, const ModelNode &node, const wrapper::CommandBuffer &cmd_buf,
                              const VkPipelineLayout layout) {
    if (!node.mesh.empty()) {
        // Pass the node's matrix via push constants
        // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
        glm::mat4 node_matrix = node.matrix;
        auto *current_parent = node.parent;

        while (current_parent) {
            node_matrix = current_parent->matrix * node_matrix;
            current_parent = current_parent->parent;
        }

        // Pass the final matrix to the vertex shader using push constants
        cmd_buf.push_constants<glm::mat4>(&node_matrix, layout);

        for (const auto &primitive : node.mesh) {
            if (primitive.index_count() > 0) {
                cmd_buf.draw_indexed(primitive.index_count(), primitive.first_index());
            }
        }
    }

    for (const auto &child_node : node.children) {
        draw_node(model, child_node, cmd_buf, layout);
    }
}

void ModelRenderer::setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer,
                                const TextureResource *depth_buffer, const std::vector<wrapper::Shader> &shaders,
                                const ModelGpuData &model) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);
    assert(!shaders.empty());

    // TODO: Can we turn this into one builder pattern call?
    auto *gltf_stage = render_graph->add<GraphicsStage>("glTF2 model");

    gltf_stage->set_depth_options(true, true)
        ->uses_shaders(shaders)
        ->bind_buffer(model.vertex_buffer(), 0)
        ->bind_buffer(model.index_buffer(), 0)
        ->writes_to(back_buffer)
        ->writes_to(depth_buffer)
        ->reads_from(model.vertex_buffer())
        ->reads_from(model.index_buffer())
        ->add_push_constant_range(sizeof(glm::mat4))
        ->add_descriptor_layout(model.descriptor_layout())
        ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
            for (const auto &node : model.nodes()) {
                cmd_buf.bind_descriptor(model.descriptor_set(), physical.pipeline_layout());
                draw_node(model, node, cmd_buf, physical.pipeline_layout());
            }
        });
}

} // namespace inexor::vulkan_renderer::gltf
