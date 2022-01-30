#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/gltf/cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/gltf/vertex.hpp"
#include "inexor/vulkan-renderer/pbr/pbr_shader_params.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

namespace inexor::vulkan_renderer::skybox {

class SkyboxGpuData : public gltf::ModelGpuPbrDataBase {
private:
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

public:
    SkyboxGpuData(RenderGraph *render_graph, const gltf::ModelCpuData &skybox_model,
                  const cubemap::GpuCubemap &skybox_texture, const wrapper::UniformBuffer<SkyboxUBO> &skybox_matrices,
                  const wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO> &pbr_parameters, std::string name);

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set;
    }
};

} // namespace inexor::vulkan_renderer::skybox
