#include "inexor/vulkan-renderer/gltf/pbr_renderer.hpp"

#include "inexor/vulkan-renderer/gltf/material.hpp"

#include "glm/vec4.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

ModelPbrRenderer::ModelPbrRenderer(const wrapper::Device &device)
    : m_shader_loader(device, m_shader_files, "gltf pbr") {}

void ModelPbrRenderer::render_node(const ModelNode &node, const VkDescriptorSet scene_descriptor_set,
                                   const wrapper::CommandBuffer &cmd_buf, const VkPipelineLayout pipeline_layout,
                                   const AlphaMode &alpha_mode) {
    if (node.mesh) {
        for (const auto &primitive : node.mesh->primitives) {

            if (primitive.material.alpha_mode == alpha_mode) {

                const std::vector<VkDescriptorSet> descriptorsets = {
                    scene_descriptor_set, primitive.material.descriptor_set, node.mesh->ubo->descriptor_set};

                // cmd_buf.bind_descriptors(descriptorsets, pipeline_layout);
                cmd_buf.push_constants(MaterialPushConstBlock(primitive.material), pipeline_layout);

                if (primitive.index_count > 0) {
                    cmd_buf.draw_indexed(primitive.index_count, primitive.first_index);
                } else {
                    cmd_buf.draw(primitive.vertex_count);
                }
            }
        }
    }

    for (const auto &child : node.children) {
        render_node(child, scene_descriptor_set, cmd_buf, pipeline_layout, alpha_mode);
    }
}

void ModelPbrRenderer::render_model(const std::vector<ModelNode> &nodes, const VkDescriptorSet scene_descriptor_set,
                                    const wrapper::CommandBuffer &cmd_buf, const VkPipelineLayout pipeline_layout) {

    for (const auto &node : nodes) {
        render_node(node, scene_descriptor_set, cmd_buf, pipeline_layout, AlphaMode::ALPHAMODE_OPAQUE);
    }
    for (const auto &node : nodes) {
        render_node(node, scene_descriptor_set, cmd_buf, pipeline_layout, AlphaMode::ALPHAMODE_MASK);
    }

    // TODO: Isn't it possible to pre-assign the model nodes into vectors so we don't iterate unnecessarily?
    // TODO: Render transparent primitives using AlphaMode::ALPHAMODE_BLEND!
}

void ModelPbrRenderer::setup_stage(RenderGraph *render_graph, const VkDescriptorSet scene,
                                   const TextureResource *back_buffer, const TextureResource *depth_buffer,
                                   const ModelGpuPbrData &model) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);

    auto *gltf_stage = render_graph->add<GraphicsStage>("gltf2 model");

    // Only bind an index buffer if available
    if (model.index_buffer() != nullptr) {
        gltf_stage->reads_from(model.index_buffer());
    }

#if 0
    // TODO: This crashes during swapchain recreation!
    gltf_stage->set_depth_options(true, true)
        ->uses_shaders(m_shader_loader.shaders())
        ->bind_buffer(model.vertex_buffer(), 0) // TODO: Unify bind_buffer with reads_from?
        ->bind_buffer(model.index_buffer(), 0)
        ->writes_to(back_buffer)
        ->writes_to(depth_buffer)
        ->reads_from(model.vertex_buffer())
        ->add_push_constant_range<glm::mat4>()
        ->add_descriptor_layout(model.descriptor_set_layout)
        ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
            cmd_buf.bind_descriptor(model.descriptor_set, physical.pipeline_layout());
            render_model(model.nodes(), model.descriptor_set, cmd_buf, physical.pipeline_layout());
        });
#endif
}

} // namespace inexor::vulkan_renderer::gltf
