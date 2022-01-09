#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"

namespace inexor::vulkan_renderer::skybox {

SkyboxGpuData::SkyboxGpuData(RenderGraph *render_graph, const gltf::ModelCpuData &model,
                             const texture::GpuTexture &skybox_texture)
    : ModelGpuPbrDataBase(render_graph->device_wrapper(), model.model()) {

    load_textures();
    load_materials();
    load_nodes();
    load_animations();
    load_skins();

    setup_rendering_resources(render_graph, skybox_texture);
}

void SkyboxGpuData::setup_rendering_resources(RenderGraph *render_graph, const texture::GpuTexture &skybox_texture) {

    m_vertex_buffer = render_graph->add<BufferResource>("skybox vertices", BufferUsage::VERTEX_BUFFER)
                          ->set_vertex_attribute_layout<gltf::ModelVertex>(gltf::ModelVertex::vertex_attribute_layout())
                          ->upload_data(vertices());

    m_index_buffer =
        render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)->upload_data(indices());

    m_skybox_uniform_buffer =
        std::make_unique<wrapper::UniformBuffer<ModelMatrices>>(render_graph->device_wrapper(), "skybox");

    m_params_uniform_buffer =
        std::make_unique<wrapper::UniformBuffer<ShaderValuesParams>>(render_graph->device_wrapper(), "skybox");
}

} // namespace inexor::vulkan_renderer::skybox
