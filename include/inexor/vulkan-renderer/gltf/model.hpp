#pragma once

#include "inexor/vulkan-renderer/gltf/model_animation.hpp"
#include "inexor/vulkan-renderer/gltf/model_file.hpp"
#include "inexor/vulkan-renderer/gltf/model_material.hpp"
#include "inexor/vulkan-renderer/gltf/model_node.hpp"
#include "inexor/vulkan-renderer/gltf/texture_sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

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

/// @brief A struct for glTF2 model vertices.
struct ModelVertex {
    ModelVertex() = default;

    /// @brief Overloaded constructor.
    /// @param position The position of the model vertex
    /// @param color_rgb The color of the model vertex
    ModelVertex(const glm::vec3 position, const glm::vec3 color_rgb) : pos(position), color(color_rgb) {}

    glm::vec3 pos{};
    glm::vec3 color{};
    glm::vec3 normal{};
    std::array<glm::vec2, 2> uv{};
    glm::vec4 joint;
    glm::vec4 weight;
};

/// @brief A wrapper class for glTF2 models.
/// Loading the glTF2 file is separated from parsing its data.
/// This allows for better task-based parallelization.
class Model {
private:
    const tinygltf::Model &m_model;
    const wrapper::Device &m_device;
    const float m_model_scale;
    std::string m_name;

    ModelShaderData m_shader_data;

    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;
    std::vector<std::uint32_t> m_texture_indices;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelVertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<ModelAnimation> animations;
    std::unordered_set<std::string> m_unsupported_attributes;

    // Some glTF2 model files with multiple scenes have a default scene index.
    std::optional<std::uint32_t> m_default_scene_index{};

    const TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                                   VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    ///
    ///
    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    ///
    ///
    ModelNode *node_from_index(std::uint32_t index);

    /// @brief Load a glTF2 model node.
    /// @param parent The parent node. If no parent exists this will be ``nullptr``
    /// @param start_node The node to begin with
    /// @param scene_index
    /// @param node_index
    void load_node(ModelNode *parent, const tinygltf::Node &start_node, std::uint32_t scene_index,
                   std::uint32_t node_index);

    ///  @brief
    void load_materials();

    ///  @brief
    void load_animations();

    ///  @brief
    void load_textures();

    ///  @brief
    void load_nodes();

    // TODO: load animations
    // TODO: load animation skins
    // TODO: load pbr (physically based rendering) settings
    // TODO: load multiple texture coordinate sets

public:
    /// @brief Overloaded constructor which accepts ModelFile as argument
    /// @paran device The device wrapper
    /// @param model_file The glTF2 model file
    ///
    ///
    Model(const wrapper::Device &device, const ModelFile &model_file, float scale, glm::mat4 projection,
          glm::mat4 model);

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

    void update_matrices(glm::mat4 projection, glm::mat4 view);
};

} // namespace inexor::vulkan_renderer::gltf
