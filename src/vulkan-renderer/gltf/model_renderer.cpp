#include "inexor/vulkan-renderer/gltf/model_renderer.hpp"

#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

ModelRenderer::ModelRenderer(RenderGraph *render_graph, const TextureResource *back_buffer,
                             const TextureResource *depth_buffer, const std::vector<wrapper::Shader> &shaders,
                             wrapper::DescriptorBuilder &descriptor_builder)
    : m_render_graph(render_graph), m_back_buffer(back_buffer), m_depth_buffer(depth_buffer), m_shaders(shaders),
      m_descriptor_builder(descriptor_builder) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);
    assert(!shaders.empty());
}

void ModelRenderer::render_model_node(const Model &model, const wrapper::CommandBuffer &cmd_buf,
                                      const VkPipelineLayout layout, const ModelNode &node) {
    if (!node.mesh.empty()) {
        // Pass the node's matrix via push constants.
        // Traverse node hierarchy to the top-most parent to get the final matrix of the current node.
        // TODO: Implement caching for this!
        glm::mat4 node_matrix = node.matrix;
        auto *current_parent = node.parent;

        while (current_parent != nullptr) {
            node_matrix = current_parent->matrix * node_matrix;
            current_parent = current_parent->parent;
        }

        cmd_buf.push_constants(layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), &node_matrix);

        for (const auto &primitive : node.mesh) {
            if (primitive.index_count > 0) {
                const auto &texture_index = model.material(primitive.material_index).base_color_texture_index;
                cmd_buf.bind_descriptor(m_descriptors[/* texture_index*/ 0], layout);
                cmd_buf.draw_indexed(primitive.index_count);
            }
        }
    }

    for (const auto &child_node : node.children) {
        render_model_node(model, cmd_buf, layout, child_node);
    }
}

void ModelRenderer::render_model_nodes(const Model &model, const wrapper::CommandBuffer &cmd_buf,
                                       const VkPipelineLayout layout) {
    // Render all nodes of the glTF model recursively.
    for (const auto &node : model.nodes()) {
        render_model_node(model, cmd_buf, layout, node);
    }
}

void ModelRenderer::render_model(const Model &model, const std::size_t scene_index,
                                 const wrapper::UniformBuffer &uniform_buffer) {
    m_gltf_vertex_buffer = m_render_graph->add<BufferResource>("gltf vertex buffer", BufferUsage::VERTEX_BUFFER);
    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, pos)); // NOLINT
    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT,
                                               offsetof(gltf::ModelVertex, color)); // NOLINT
    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT,
                                               offsetof(gltf::ModelVertex, normal));                         // NOLINT
    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, uv)); // NOLINT
    m_gltf_vertex_buffer->upload_data(model.scene_vertices(scene_index));

    m_gltf_index_buffer = m_render_graph->add<BufferResource>("gltf index buffer", BufferUsage::INDEX_BUFFER);
    m_gltf_index_buffer->upload_data(model.scene_indices(scene_index));

    m_descriptors = m_descriptor_builder.add_uniform_buffer<ModelShaderData>(uniform_buffer.buffer())
                        .add_combined_image_sampler(model.texture(0)) // TODO: Change me pls!
                        .build("glTF2 model");

    auto *gltf_stage = m_render_graph->add<GraphicsStage>("gltf stage");
    gltf_stage->writes_to(m_back_buffer);
    gltf_stage->writes_to(m_depth_buffer);
    gltf_stage->reads_from(m_gltf_index_buffer);
    // TODO: Add rendergraph validation warning if no index buffer is bound!
    gltf_stage->reads_from(m_gltf_vertex_buffer);
    gltf_stage->bind_buffer(m_gltf_vertex_buffer, 0);
    gltf_stage->set_clears_screen(true);
    gltf_stage->set_depth_options(true, true);

    for (const auto &shader : m_shaders) {
        gltf_stage->uses_shader(shader);
    }

    for (const auto &descriptor : m_descriptors) {
        gltf_stage->add_descriptor_layout(descriptor.descriptor_set_layout());
    }

    // Add a push constant range for the model matrix.
    gltf_stage->add_push_constant_range(sizeof(glm::mat4));

    gltf_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        render_model_nodes(model, cmd_buf, physical.pipeline_layout());
    });
}

} // namespace inexor::vulkan_renderer::gltf
