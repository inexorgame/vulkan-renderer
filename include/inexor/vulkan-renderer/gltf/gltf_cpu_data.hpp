#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_animation.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_file.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_vertex.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <unordered_set>

namespace inexor::vulkan_renderer::gltf {

class ModelCpuData {
private:
    std::string m_name;
    float m_model_scale{1.0f};

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<std::uint32_t> m_indices;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelNode> m_linear_nodes;
    std::vector<ModelVertex> m_vertices;
    std::vector<ModelAnimation> animations;
    std::vector<ModelSkin> m_skins;

    // TODO: Oh oh. Shouldn't this be part of the GpuData part?
    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;

    // A glTF2 model file can contain arbitrary node types such as cameras and lights
    // We do not support all possible node types (yet)
    std::unordered_map<std::string, bool> m_unsupported_node_types{};

    // Some glTF2 model files with multiple scenes have a default scene index
    std::optional<std::uint32_t> m_default_scene_index{};

    // In case the model contains textures but no default texture sampler, use this one.
    TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                             VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    ModelNode *node_from_index(std::uint32_t index);

    void load_node(const wrapper::Device &device_wrapper, const tinygltf::Model &model, ModelNode *parent,
                   const tinygltf::Node &start_node, std::uint32_t scene_index, std::uint32_t node_index);

    void load_materials(const tinygltf::Model &model);

    void load_animations(const tinygltf::Model &model);

    void load_textures(const wrapper::Device &device, const tinygltf::Model &model);

    void load_skins(const tinygltf::Model &model);

    void load_nodes(const wrapper::Device &device_wrapper, const tinygltf::Model &model);

public:
    ModelCpuData(const wrapper::Device &device_wrapper, const ModelFile &model_file);

    ModelCpuData(const ModelCpuData &) = delete;
    ModelCpuData(ModelCpuData &&) noexcept;

    ModelCpuData &operator=(const ModelCpuData &) = delete;
    ModelCpuData &operator=(ModelCpuData &&) = default;

    [[nodiscard]] const auto &nodes() const {
        return m_nodes;
    }

#if 0
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
#endif
};

} // namespace inexor::vulkan_renderer::gltf
