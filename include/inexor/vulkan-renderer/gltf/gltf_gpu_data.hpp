#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_vertex.hpp"
#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

// TODO: Where to move this?
struct ModelMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    glm::vec3 camPos;
};

// TODO: Where to move this?
struct ModelShaderParams {
    glm::vec4 lightDir;
    float exposure = 4.5f;
    float gamma = 2.2f;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient = 1.0f;
    float debugViewInputs = 0;
    float debugViewEquation = 0;
};

/// A base class for the gpu data of glTF2 models
/// @note Initially it was planned to put ``m_vertices`` and ``m_indices`` together with their corresponding get
/// methods into a separate template base class. However this is not a good approach, since the vertices are
/// sometimes filled out by the cpu data class, but sometimes by the gpu data class - as in this example. This means
/// some gpu data classes would need to inherit from a base class named something like CpuDataBase which is
/// misleading. Furthermore it's just not worth to create a template base just for those two members and their get
/// methods. In the future, we might implement rendering of data structures which require more than one vector of
/// vertices. so it's better to keep it in the gpu data class.
class ModelGpuData : public GpuDataBase<ModelVertex, std::uint32_t> {
private:
    // TODO: Move this to GltfDataParserBase
    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    ModelNode *node_from_index(std::uint32_t index);

    void load_node(const wrapper::Device &device_wrapper, const tinygltf::Model &model, ModelNode *parent,
                   const tinygltf::Node &start_node, std::uint32_t scene_index, std::uint32_t node_index);

    void load_materials(const tinygltf::Model &model);

    void load_animations(const tinygltf::Model &model);

    void load_textures(const wrapper::Device &device, const tinygltf::Model &model);

    void load_skins(const tinygltf::Model &model);

    void load_nodes(const wrapper::Device &device_wrapper, const tinygltf::Model &model);

    std::string m_name;

    float m_model_scale{1.0f};

    // A glTF2 model file can contain arbitrary node types such as cameras and lights
    // We do not support all possible node types (yet)
    std::unordered_map<std::string, bool> m_unsupported_node_types{};

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelNode> m_linear_nodes;
    std::vector<ModelAnimation> animations;
    std::vector<ModelSkin> m_skins;

    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;

    // -------------------------------------------------------

    ModelMatrices m_scene;
    ModelShaderParams m_shader_values;

    std::unique_ptr<wrapper::UniformBuffer<ModelShaderParams>> m_uniform_buffer;

    // In case the model contains textures but no default texture sampler, use this one.
    TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                             VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    void setup_rendering_resources(RenderGraph *render_graph);

protected:
    std::vector<ModelVertex> m_vertices{};
    std::vector<std::uint32_t> m_indices{};

public:
    ModelGpuData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data, const glm::mat4 &model_matrix,
                 const glm::mat4 &proj_matrix);

    ModelGpuData(const ModelGpuData &) = delete;
    ModelGpuData(ModelGpuData &&) noexcept;

    ModelGpuData &operator=(const ModelGpuData &) = delete;
    ModelGpuData &operator=(ModelGpuData &&) = default;

    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const auto &indices() const {
        return m_indices;
    };

    [[nodiscard]] auto vertex_count() const {
        return m_vertices.size();
    }

    [[nodiscard]] auto index_count() const {
        return m_indices.size();
    }
};

} // namespace inexor::vulkan_renderer::gltf
