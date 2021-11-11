#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_cpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include "glm/vec3.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

struct ModelMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    glm::vec3 camPos;
};

struct ModelShaderParams {
    glm::vec4 lightDir;
    float exposure = 4.5f;
    float gamma = 2.2f;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient = 1.0f;
    float debugViewInputs = 0;
    float debugViewEquation = 0;
};

/// @brief A wrapper class for glTF2 models.
/// Loading the glTF2 file is separated from parsing its data.
/// This allows for better task-based parallelization.
class ModelGpuData {
private:
    std::string m_name;
    float m_model_scale{1.0f};

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<std::uint32_t> m_indices;
    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;

    ModelMatrices m_scene;
    ModelShaderParams m_shader_values;

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<wrapper::UniformBuffer<ModelShaderParams>> m_uniform_buffer;

    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

    // Some glTF2 model files with multiple scenes have a default scene index.
    std::optional<std::uint32_t> m_default_scene_index{};

    // In case the model contains textures but no default texture sampler, use this one.
    TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                             VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    void setup_rendering_resources(RenderGraph *render_graph, const ModelCpuData &model_cpu_data,
                                   const glm::mat4 &model_matrix, const glm::mat4 &proj_matrix);

public:
    ModelGpuData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data, const glm::mat4 &model_matrix,
                 const glm::mat4 &proj_matrix);

    ModelGpuData(const ModelGpuData &) = delete;
    ModelGpuData(ModelGpuData &&) noexcept;

    ModelGpuData &operator=(const ModelGpuData &) = delete;
    ModelGpuData &operator=(ModelGpuData &&) = default;

    [[nodiscard]] auto vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] auto index_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] auto descriptor_layout() const {
        return m_descriptor->descriptor_set_layout();
    }
};

} // namespace inexor::vulkan_renderer::gltf
