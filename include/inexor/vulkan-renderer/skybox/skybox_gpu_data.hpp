#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <glm/mat4x4.hpp>

namespace inexor::vulkan_renderer::skybox {

class SkyboxGpuData {
private:
    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

    std::uint32_t m_vertex_count{0};
    std::uint32_t m_index_count{0};

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    // TODO: Move into one base class?
    struct UBOMatrices {
        glm::mat4 projection;
        glm::mat4 model;
        glm::mat4 view;
        glm::vec3 camPos;
    };

    std::unique_ptr<wrapper::UniformBuffer<UBOMatrices>> m_skybox_uniform_buffer;

    struct ShaderValuesParams {
        glm::vec4 lightDir;
        float exposure = 4.5f;
        float gamma = 2.2f;
        float prefilteredCubeMipLevels;
        float scaleIBLAmbient = 1.0f;
        float debugViewInputs = 0;
        float debugViewEquation = 0;
    };

    std::unique_ptr<wrapper::UniformBuffer<ShaderValuesParams>> m_params_uniform_buffer;

public:
    ///
    ///
    ///
    ///
    ///
    SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph,
                  const gltf::ModelGpuPbrData &model_data, const texture::GpuTexture &skybox_texture);

    [[nodiscard]] const BufferResource *vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] const BufferResource *index_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] std::uint32_t vertex_count() const {
        return m_vertex_count;
    }

    [[nodiscard]] std::uint32_t index_count() const {
        return m_index_count;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }
};

} // namespace inexor::vulkan_renderer::skybox
