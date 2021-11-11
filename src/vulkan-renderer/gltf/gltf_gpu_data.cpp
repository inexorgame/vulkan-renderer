#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::gltf {

ModelGpuData::ModelGpuData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data, const glm::mat4 &model_matrix,
                           const glm::mat4 &proj_matrix)
    : m_model_cpu_data(model_cpu_data), m_model_matrix(model_matrix), m_proj_matrix(proj_matrix) {
    setup_rendering_resources(render_graph);
}

ModelGpuData::ModelGpuData(ModelGpuData &&other) noexcept : m_model_cpu_data(other.m_model_cpu_data) {
    m_name = std::move(other.m_name);
    m_model_scale = other.m_model_scale;
    m_texture_indices = std::move(other.m_texture_indices);
    m_indices = std::move(other.m_indices);
    m_textures = std::move(other.m_textures);
    m_texture_samplers = std::move(other.m_texture_samplers);
    m_scene = std::move(other.m_scene);
    m_shader_values = std::move(other.m_shader_values);
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, nullptr);
    m_descriptor = std::exchange(other.m_descriptor, nullptr);
    m_uniform_buffer = std::exchange(other.m_uniform_buffer, nullptr);
    m_vertex_buffer = std::exchange(other.m_vertex_buffer, nullptr);
    m_index_buffer = std::exchange(other.m_index_buffer, nullptr);
    m_default_scene_index = other.m_default_scene_index;
}

void ModelGpuData::setup_rendering_resources(RenderGraph *render_graph) {

#if 0
    // TODO: Make this into a template so the upcoming calls are easier?
    // m_vertex_buffer = render_graph->add<BufferResource, gltf::ModelVertex>("gltf vertex buffer",
    // BufferUsage::VERTEX_BUFFER);
    m_vertex_buffer = render_graph->add<BufferResource>("gltf vertex buffer", BufferUsage::VERTEX_BUFFER);

    // TODO: Can we turn this into a template or some container format?
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, pos))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, normal))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, uv0))
        ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(gltf::ModelVertex, color))
        ->set_element_size<gltf::ModelVertex>()
        ->upload_data(m_vertices);

    m_index_buffer = render_graph->add<BufferResource>("gltf index buffer", BufferUsage::INDEX_BUFFER);
    m_index_buffer->upload_data(m_indices);

    // TODO: Update for glTF2 PBR rendering!
    const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

    // TODO: Move this into rendergraph!
    m_descriptor_pool = std::make_unique<wrapper::DescriptorPool>(device, pool_sizes, "gltf descriptor pool");

    wrapper::DescriptorBuilder builder(device, m_descriptor_pool->descriptor_pool());

    // TODO: Move this into rendergraph!
    m_uniform_buffer = std::make_unique<wrapper::UniformBuffer<ModelShaderParams>>(device, "gltf uniform buffer");

    ModelShaderParams shader_data;

    m_scene.model = model_matrix;
    m_scene.projection = proj_matrix;

    // TODO: This data is still empty at this point!!
    m_uniform_buffer->update(&shader_data);

    m_descriptor =
        builder.add_uniform_buffer<ModelShaderParams>(m_uniform_buffer->buffer()).build("gltf uniform buffer object");
#endif
}

} // namespace inexor::vulkan_renderer::gltf
