#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::skybox {

SkyboxGpuData::SkyboxGpuData(RenderGraph *render_graph, const gltf::ModelCpuData &skybox_model,
                             const cubemap::GpuCubemap &skybox_texture,
                             const wrapper::UniformBuffer<SkyboxUBO> &skybox_matrices,
                             const wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO> &pbr_parameters)
    : ModelGpuPbrDataBase(render_graph->device_wrapper(), skybox_model.model()) {

    assert(render_graph);

    // Use the base class methods to laod the essential data from the gltf model. However we do not load animations or
    // skins for the skybox and we set up the rendering resources in a different way compared with a gltf pbr model.
    load_textures();
    load_materials();
    load_nodes();

    create_vertex_buffer(render_graph, gltf::ModelVertex::vertex_attribute_layout());
    create_index_buffer(render_graph);

    auto builder = wrapper::DescriptorBuilder(render_graph->device_wrapper());

    m_descriptor =
        builder.add_uniform_buffer(skybox_matrices, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_uniform_buffer(pbr_parameters, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_combined_image_sampler(skybox_texture)
            .build("skybox");
}

} // namespace inexor::vulkan_renderer::skybox
