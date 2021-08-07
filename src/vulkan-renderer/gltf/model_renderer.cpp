#include "inexor/vulkan-renderer/gltf/model_renderer.hpp"

#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

ModelRenderer::ModelRenderer(RenderGraph *render_graph, const TextureResource *back_buffer,
                             const TextureResource *depth_buffer, const wrapper::Shader &vertex_shader,
                             const wrapper::Shader &fragment_shader_texture,
                             const wrapper::Shader &fragment_shader_color)
    : m_render_graph(render_graph), m_back_buffer(back_buffer), m_depth_buffer(depth_buffer),
      m_vertex_shader(vertex_shader), m_fragment_shader_texture(fragment_shader_texture),
      m_fragment_shader_color(fragment_shader_color) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);
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
                if (model.texture_count() > 0) {
                    // Only attempt to render textures if model has textures.
                    const auto &texture_index = model.material(primitive.material_index).base_color_texture_index;
                    cmd_buf.bind_descriptor(*m_texture_descriptors[texture_index], 1, layout);
                }

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
    for (const auto &node : model.nodes()) {
        render_model_node(model, cmd_buf, layout, node);
    }
}

void ModelRenderer::render_model(const wrapper::Device &device, const Model &model, const std::size_t scene_index,
                                 const wrapper::UniformBuffer &uniform_buffer) {

    m_gltf_vertex_buffer = m_render_graph->add<BufferResource>("gltf vertex buffer", BufferUsage::VERTEX_BUFFER);

    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, pos))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, color))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, normal))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, uv))
        ->upload_data(model.scene_vertices(scene_index));

    m_gltf_index_buffer = m_render_graph->add<BufferResource>("gltf index buffer", BufferUsage::INDEX_BUFFER);

    m_gltf_index_buffer->upload_data(model.scene_indices(scene_index));

    std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

    // Only append textures if model has textures.
    if (model.texture_count() > 0) {
        pool_sizes.push_back(
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<std::uint32_t>(model.texture_count())});
    }

    m_descriptor_pool = std::make_unique<wrapper::DescriptorPool>(device, pool_sizes, "glTF model");

    m_descriptor_builder = std::make_unique<wrapper::DescriptorBuilder>(device, m_descriptor_pool->descriptor_pool());

    m_descriptor_ubo =
        m_descriptor_builder->add_uniform_buffer<ModelShaderData>(uniform_buffer.buffer()).build("glTF2 model UBO");

    for (const auto &texture : model.textures()) {
        m_texture_descriptors.push_back(
            std::move(m_descriptor_builder->add_combined_image_sampler(texture).build("glTF2 model texture")));
    }

    // TODO: Can we turn this into one builder pattern call?
    auto *gltf_stage = m_render_graph->add<GraphicsStage>("gltf stage");

    gltf_stage->uses_shader(m_vertex_shader)
        ->bind_buffer(m_gltf_vertex_buffer, 0)
        ->set_depth_options(true, true)
        ->writes_to(m_back_buffer)
        ->writes_to(m_depth_buffer)
        ->reads_from(m_gltf_index_buffer)
        ->reads_from(m_gltf_vertex_buffer)
        ->add_descriptor_layout(*m_descriptor_ubo);

    if (model.texture_count() > 0) {
        gltf_stage->uses_shader(m_fragment_shader_texture);
    } else {
        gltf_stage->uses_shader(m_fragment_shader_color);
    }

    for (const auto &descriptor : m_texture_descriptors) {
        gltf_stage->add_descriptor_layout(*descriptor);
    }

    gltf_stage->add_push_constant_range(sizeof(glm::mat4));

    gltf_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(*m_descriptor_ubo, 0, physical.pipeline_layout());
        render_model_nodes(model, cmd_buf, physical.pipeline_layout());
    });
}

} // namespace inexor::vulkan_renderer::gltf
