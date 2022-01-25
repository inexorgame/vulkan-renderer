#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
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

struct ModelMatrices {
    glm::mat4 projection;
    glm::mat4 model;
};

class SkyboxGpuData : public gltf::ModelGpuPbrDataBase {
private:
    struct ShaderValuesParams {
        glm::vec4 lightDir = glm::vec4(0.73994f, 0.64279f, 0.19827f, 0.0f);
        float exposure = 4.5f;
        float gamma = 2.2f;
        float prefilteredCubeMipLevels = 0;
        float scaleIBLAmbient = 1.0f;
        float debugViewInputs = 0;
        float debugViewEquation = 0;
    };

    ShaderValuesParams m_default_shader_params;

    std::unique_ptr<wrapper::UniformBuffer<ShaderValuesParams>> m_params_ubo;
    std::unique_ptr<wrapper::UniformBuffer<ModelMatrices>> m_skybox_ubo;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    void setup_rendering_resources(RenderGraph *render_graph, const cubemap::GpuCubemap &skybox_texture);

public:
    SkyboxGpuData(RenderGraph *render_graph, const gltf::ModelCpuData &model,
                  const cubemap::GpuCubemap &skybox_texture);

    void update_uniform_buffer(const ModelMatrices *data) {
        assert(data);
        m_skybox_ubo->update(data);
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }
};

} // namespace inexor::vulkan_renderer::skybox
