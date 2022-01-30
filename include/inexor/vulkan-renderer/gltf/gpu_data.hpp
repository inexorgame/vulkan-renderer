#pragma once

#include "inexor/vulkan-renderer/gltf/cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/pbr/pbr_shader_params.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::gltf {

class ModelGpuPbrData : public ModelGpuPbrDataBase {
private:
    const wrapper::Device &m_device;

    std::string m_name;
    float m_model_scale{1.0f};

    // Start of possible garbage
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSet m_scene_descriptor_set{VK_NULL_HANDLE};
    // End of possible garbage: Do we really need those members?

    void setup_node_descriptor_sets(VkDevice device, const ModelNode &node);

    void setup_rendering_resources(RenderGraph *render_graph,
                                   const wrapper::UniformBuffer<DefaultUBO> &shader_data_model,
                                   const wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO> &shader_data_pbr,
                                   VkDescriptorImageInfo irradiance_cube_texture,
                                   VkDescriptorImageInfo prefiltered_cube_texture,
                                   VkDescriptorImageInfo brdf_lut_texture);

public:
    VkDescriptorSetLayout m_scene_descriptor_set_layout;
    VkDescriptorSetLayout m_material_descriptor_set_layout;
    VkDescriptorSetLayout m_node_descriptor_set_layout;

    ModelGpuPbrData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data,
                    const wrapper::UniformBuffer<DefaultUBO> &shader_data_model,
                    const wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO> &shader_data_pbr,
                    VkDescriptorImageInfo irradiance_cube_texture, VkDescriptorImageInfo prefiltered_cube_texture,
                    VkDescriptorImageInfo brdf_lut_texture, std::string name);

    ModelGpuPbrData(const ModelGpuPbrData &) = delete;
    ModelGpuPbrData(ModelGpuPbrData &&) noexcept;

    ~ModelGpuPbrData();

    ModelGpuPbrData &operator=(const ModelGpuPbrData &) = delete;
    ModelGpuPbrData &operator=(ModelGpuPbrData &&) noexcept = default;

    [[nodiscard]] VkDescriptorSet scene_descriptor_set() const {
        return m_scene_descriptor_set;
    }
};

} // namespace inexor::vulkan_renderer::gltf
