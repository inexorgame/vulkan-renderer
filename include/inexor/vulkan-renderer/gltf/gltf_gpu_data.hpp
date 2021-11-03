#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_animation.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_file.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

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
public:
    ModelMatrices m_scene;
    ModelMatrices m_skybox;
    ModelShaderParams m_shader_values;

private:
    std::string m_name;
    const float m_model_scale{1.0f};

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<std::uint32_t> m_indices;
    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelNode> m_linear_nodes;
    std::vector<ModelVertex> m_vertices;
    std::vector<ModelAnimation> animations;
    std::vector<ModelSkin> m_skins;

    // The glTF2 model file can contain material information.
    // We store all unsupported material features in this unordered map so we can print it in the console after the
    // model has been loaded and parsed.
    std::unordered_set<std::string> m_unsupported_attributes;

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<wrapper::UniformBuffer> m_uniform_buffer;

    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

    // Some glTF2 model files with multiple scenes have a default scene index.
    std::optional<std::uint32_t> m_default_scene_index{};

    // In case the model contains textures but no default texture sampler, use this one.
    const TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                                   VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    /// @brief Find a node by parent and index in the parent node's children.
    /// @param parent The node's parent
    /// @param index The ModelNode's index in m_nodes
    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    /// @brief Finds a node by index
    /// @param index The ModelNode's index in m_nodes
    ModelNode *node_from_index(std::uint32_t index);

    void load_node(const wrapper::Device &device_wrapper, const tinygltf::Model &model, ModelNode *parent,
                   const tinygltf::Node &start_node, std::uint32_t scene_index, std::uint32_t node_index);

    // We pass the const reference to all other methods because we don't want to store the model as const reference for
    // the entire lifetime of this class.

    void load_materials(const tinygltf::Model &model);

    void load_animations(const tinygltf::Model &model);

    void load_textures(const wrapper::Device &device, const tinygltf::Model &model);

    void load_skins(const tinygltf::Model &model);

    void load_nodes(const wrapper::Device &device_wrapper, const tinygltf::Model &model);

    void setup_rendering_resources(const wrapper::Device &device_wrapper, RenderGraph *render_graph,
                                   glm::mat4 model_matrix, glm::mat4 proj_matrix);

public:
    glm::mat4 aabb;

    ModelGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph, const ModelFile &model_file,
                 glm::mat4 model_matrix, glm::mat4 proj);

    ModelGpuData(const ModelGpuData &) = delete;
    ModelGpuData(ModelGpuData &&) noexcept;
    ~ModelGpuData() = default;

    ModelGpuData &operator=(const ModelGpuData &) = delete;
    ModelGpuData &operator=(ModelGpuData &&) = default;

    [[nodiscard]] std::size_t texture_count() const {
        return m_textures.size();
    }

    [[nodiscard]] std::size_t texture_index_count() const {
        return m_texture_indices.size();
    }

    [[nodiscard]] std::size_t material_count() const {
        return m_materials.size();
    }

    [[nodiscard]] std::size_t node_count() const {
        return m_nodes.size();
    }

    [[nodiscard]] const auto &nodes() const {
        return m_nodes;
    }

    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const auto &indices() const {
        return m_indices;
    }

    [[nodiscard]] const wrapper::GpuTexture &texture(const std::size_t texture_index) const {
        // TODO: Create an error texture or throw an exception in case of invalid texture access?
        assert(texture_index < m_textures.size());
        return m_textures.at(texture_index);
    }

    [[nodiscard]] const std::vector<wrapper::GpuTexture> &textures() const {
        return m_textures;
    }

    [[nodiscard]] const ModelMaterial &material(const std::size_t material_index) const {
        // TODO: Throw an exception in case of invalid access?
        if (material_index < m_materials.size()) {
            return m_materials.at(0);
        }
        return m_materials.at(material_index);
    }

    // TODO: not const by intention?
    [[nodiscard]] auto materials() const {
        return m_materials;
    }

    [[nodiscard]] std::optional<std::uint32_t> default_scene_index() const {
        return m_default_scene_index;
    }

    // TODO: Do not return const reference to unique_ptr!
    [[nodiscard]] const auto &index_buffer() const {
        return m_index_buffer;
    }

    // TODO: Do not return const reference to unique_ptr!
    [[nodiscard]] const auto &vertex_buffer() const {
        return m_vertex_buffer;
    }

    // TODO: Do not return const reference to unique_ptr!
    [[nodiscard]] const auto &ubo() const {
        return m_uniform_buffer;
    }

    [[nodiscard]] const auto &descriptor_layout() const {
        return m_descriptor->descriptor_set_layout();
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }

    void update_matrices(glm::mat4 projection, glm::mat4 view);
};

} // namespace inexor::vulkan_renderer::gltf
