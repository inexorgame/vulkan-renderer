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

void ModelRenderer::render_model(const Model &model, const std::size_t scene_index) {
    m_gltf_index_buffer = m_render_graph->add<BufferResource>("gltf index buffer", BufferUsage::INDEX_BUFFER);

    m_gltf_index_buffer->upload_data(model.scene_indices(scene_index));

    m_gltf_vertex_buffer = m_render_graph->add<BufferResource>("gltf vertex buffer", BufferUsage::VERTEX_BUFFER);

    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, pos)); // NOLINT
    m_gltf_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT,
                                               offsetof(gltf::ModelVertex, color)); // NOLINT

    m_gltf_vertex_buffer->upload_data(model.scene_vertices(scene_index));

    auto *gltf_stage = m_render_graph->add<GraphicsStage>("gltf stage");

    gltf_stage->writes_to(m_back_buffer);
    gltf_stage->writes_to(m_depth_buffer);
    gltf_stage->reads_from(m_gltf_index_buffer);
    gltf_stage->reads_from(m_gltf_vertex_buffer);
    gltf_stage->bind_buffer(m_gltf_vertex_buffer, 0);
    gltf_stage->set_clears_screen(true);
    gltf_stage->set_depth_options(true, true);

    const auto indices_count = model.scene_indices(scene_index).size();

    // TODO: Render model correctly
    // TODO: Build descriptors!

    gltf_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(m_descriptors[0], physical.pipeline_layout());
        cmd_buf.draw_indexed(indices_count);
    });

    for (const auto &shader : m_shaders) {
        gltf_stage->uses_shader(shader);
    }

    gltf_stage->add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
}

} // namespace inexor::vulkan_renderer::gltf
