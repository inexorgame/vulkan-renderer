#pragma once

#include "inexor/vulkan-renderer/gltf/cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::gltf {

// TODO: Where to move this?
struct ModelPbrShaderParams {
    glm::vec4 lightDir;
    float exposure = 4.5f;
    float gamma = 2.2f;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient = 1.0f;
    float debugViewInputs = 0;
    float debugViewEquation = 0;
};

class ModelGpuPbrData : public ModelGpuPbrDataBase {
private:
    const wrapper::Device &m_device;

    std::string m_name;
    float m_model_scale{1.0f};

    VkDescriptorPool m_descriptor_pool;

    VkDescriptorSet m_scene_descriptor_set{VK_NULL_HANDLE};
    VkDescriptorSet m_material_descriptor_set{VK_NULL_HANDLE};
    VkDescriptorImageInfo m_brdf_lut_texture;
    VkDescriptorImageInfo m_enviroment_cube_texture;
    VkDescriptorImageInfo m_irradiance_cube_texture;
    VkDescriptorImageInfo m_prefiltered_cube_texture;

    // TODO: What other stuff can be part of ModelGpuPbrDataBase?
    VkDescriptorSetLayout m_node_descriptor_set_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_material_descriptor_set_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_scene_descriptor_set_layout{VK_NULL_HANDLE};

    ModelPbrShaderParams m_shader_values;
    std::unique_ptr<wrapper::UniformBuffer<ModelPbrShaderParams>> m_shader_params;

    void setup_node_descriptor_sets(const ModelNode &node);

    void setup_rendering_resources(RenderGraph *render_graph);

public:
    ModelGpuPbrData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data,
                    VkDescriptorImageInfo brdf_lut_texture, VkDescriptorImageInfo enviroment_cube_texture,
                    VkDescriptorImageInfo irradiance_cube_texture, VkDescriptorImageInfo prefiltered_cube_texture,
                    const glm::mat4 &model_matrix, const glm::mat4 &proj_matrix);

    ModelGpuPbrData(const ModelGpuPbrData &) = delete;
    ModelGpuPbrData(ModelGpuPbrData &&) noexcept;

    ~ModelGpuPbrData();

    ModelGpuPbrData &operator=(const ModelGpuPbrData &) = delete;
    ModelGpuPbrData &operator=(ModelGpuPbrData &&) noexcept = default;
};

} // namespace inexor::vulkan_renderer::gltf
