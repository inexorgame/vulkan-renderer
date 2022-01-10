#pragma once

#include "inexor/vulkan-renderer/gltf/cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/gltf/vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <glm/mat4x4.hpp>

namespace inexor::vulkan_renderer::skybox {

class SkyboxGpuData : public gltf::ModelGpuPbrDataBase {
private:
    struct ShaderValuesParams {
        glm::vec4 lightDir;
        float exposure = 4.5f;
        float gamma = 2.2f;
        float prefilteredCubeMipLevels;
        float scaleIBLAmbient = 1.0f;
        float debugViewInputs = 0;
        float debugViewEquation = 0;
    };

    std::unique_ptr<wrapper::UniformBuffer<ShaderValuesParams>> m_params_ubo;

    struct ModelMatrices {
        glm::mat4 projection;
        glm::mat4 model;
        glm::mat4 view;
        glm::vec3 camera_pos;
    };

    std::unique_ptr<wrapper::UniformBuffer<ModelMatrices>> m_skybox_ubo;

    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    void setup_rendering_resources(RenderGraph *render_graph, const texture::GpuTexture &skybox_texture);

public:
    SkyboxGpuData(RenderGraph *render_graph, const gltf::ModelCpuData &model,
                  const texture::GpuTexture &skybox_texture);

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }
};

} // namespace inexor::vulkan_renderer::skybox
