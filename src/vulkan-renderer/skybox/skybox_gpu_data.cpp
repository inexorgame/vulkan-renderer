#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"

#include "inexor/vulkan-renderer/gltf/gltf_vertex.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

namespace inexor::vulkan_renderer::skybox {

SkyboxGpuData::SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph,
                             const gltf::ModelGpuPbrData &model_data, const texture::GpuTexture &skybox_texture) {

    m_vertex_buffer = render_graph->add<BufferResource>("skybox vertices", BufferUsage::VERTEX_BUFFER)
                          ->set_vertex_attribute_layout<gltf::ModelVertex>(gltf::ModelVertex::vertex_attribute_layout())
                          ->upload_data(model_data.vertices());

    m_index_buffer = render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)
                         ->upload_data(model_data.indices());

    const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
                                                       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};

    m_descriptor_pool = std::make_unique<wrapper::DescriptorPool>(render_graph->device_wrapper(), pool_sizes, "skybox");

    wrapper::DescriptorBuilder builder(render_graph->device_wrapper(), m_descriptor_pool->descriptor_pool());

    m_skybox_uniform_buffer =
        std::make_unique<wrapper::UniformBuffer<UBOMatrices>>(render_graph->device_wrapper(), "skybox");

    m_params_uniform_buffer =
        std::make_unique<wrapper::UniformBuffer<ShaderValuesParams>>(render_graph->device_wrapper(), "skybox");

    m_descriptor = builder.add_uniform_buffer<UBOMatrices>(m_skybox_uniform_buffer->buffer())
                       .add_uniform_buffer<ShaderValuesParams>(m_params_uniform_buffer->buffer())
                       .add_combined_image_sampler(skybox_texture)
                       .build("skybox");
}

} // namespace inexor::vulkan_renderer::skybox
