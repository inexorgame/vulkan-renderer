#pragma once

#include "inexor/vulkan-renderer/gltf/model_animation.hpp"
#include "inexor/vulkan-renderer/gltf/model_file.hpp"
#include "inexor/vulkan-renderer/gltf/model_material.hpp"
#include "inexor/vulkan-renderer/gltf/model_node.hpp"
#include "inexor/vulkan-renderer/gltf/model_vertex.hpp"
#include "inexor/vulkan-renderer/gltf/texture_sampler.hpp"
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

/// @brief The shader data for glTF model rendering.
struct ModelShaderData {
    glm::mat4 projection;
    glm::mat4 model;
    glm::vec4 light_position = glm::vec4(5.0f, 5.0f, -5.0f, 1.0f);
};

/// @brief A wrapper class for glTF2 models.
/// Loading the glTF2 file is separated from parsing its data.
/// This allows for better task-based parallelization.
class ModelGpuData {
private:
    std::string m_name;
    const float m_model_scale{1.0f};
    ModelShaderData m_shader_data;

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<std::uint32_t> m_indices;
    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelVertex> m_vertices;
    std::vector<ModelSkin> m_skins;
    std::vector<ModelAnimation> animations;
    std::unordered_set<std::string> m_unsupported_attributes;

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<wrapper::UniformBuffer> m_uniform_buffer;

    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

    // Some glTF2 model files with multiple scenes have a default scene index.
    std::optional<std::uint32_t> m_default_scene_index{};

    const TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                                   VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    /// @brief Find a node by parent and index in the parent node's children.
    /// @param parent The node's parent
    /// @param index The ModelNode's index in m_nodes
    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    /// @brief Finds a node by index
    /// @param index The ModelNode's index in m_nodes
    ModelNode *node_from_index(std::uint32_t index);

    ///
    void load_node(const tinygltf::Model &model, ModelNode *parent, const tinygltf::Node &start_node,
                   std::uint32_t scene_index, std::uint32_t node_index);

    // We pass the const reference to all other methods because we don't want to store the model as const reference for
    // the entire lifetime of this class.

    void load_materials(const tinygltf::Model &model);
    void load_animations(const tinygltf::Model &model);
    void load_textures(RenderGraph *render_graph, const tinygltf::Model &model);
    void load_skins(const tinygltf::Model &model);
    void load_nodes(const tinygltf::Model &model);

    void setup_rendering_resources(RenderGraph *render_graph);

public:
    ///
    ///
    ///
    ModelGpuData(RenderGraph *render_graph, const ModelFile &model_file);
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
        return m_vertices;
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

    [[nodiscard]] const ModelShaderData &shader_data() const {
        return m_shader_data;
    }

    [[nodiscard]] std::optional<std::uint32_t> default_scene_index() const {
        return m_default_scene_index;
    }

    [[nodiscard]] const auto &index_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] const auto &vertex_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] const auto &ubo() const {
        return m_uniform_buffer;
    }

    [[nodiscard]] const auto &descriptor_layout() const {
        return m_descriptor->descriptor_set_layout();
    }

    [[nodiscard]] const auto &descriptor() const {
        return m_descriptor;
    }

    void update_matrices(glm::mat4 projection, glm::mat4 view);
};

} // namespace inexor::vulkan_renderer::gltf
