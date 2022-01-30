#pragma once

#include "inexor/vulkan-renderer/gltf/animation.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/gltf/texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf/vertex.hpp"
#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include <tiny_gltf.h>

#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

[[nodiscard]] VkImageCreateInfo fill_image_ci(VkFormat format, std::uint32_t width, std::uint32_t height,
                                              std::uint32_t miplevel_count);

[[nodiscard]] VkImageViewCreateInfo fill_image_view_ci(VkImage image, VkFormat format, std::uint32_t miplevel_count);

class ModelGpuPbrDataBase : public GpuDataBase<gltf::ModelVertex, std::uint32_t> {
private:
    const tinygltf::Model &m_model;

    // A glTF2 model file can contain arbitrary node types such as cameras and lights
    // We do not support all possible node types (yet)
    std::unordered_map<std::string, bool> m_unsupported_node_types{};

    std::vector<std::uint32_t> m_texture_indices;
    std::vector<ModelMaterial> m_materials;
    std::vector<ModelNode> m_nodes;
    std::vector<ModelNode> m_linear_nodes;
    std::vector<ModelAnimation> animations;
    std::vector<ModelSkin> m_skins;

    // The glTF2 file could contain embedded textures
    std::vector<texture::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;

    const TextureSampler m_default_texture_sampler{};

    static constexpr VkFormat DEFAULT_TEXTURE_FORMAT{VK_FORMAT_R8G8B8A8_UNORM};

protected:
    const wrapper::Device &m_device;
    std::unique_ptr<texture::GpuTexture> m_empty_texture;
    std::unique_ptr<wrapper::UniformBuffer<DefaultUBO>> m_scene_matrices;

    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    ModelNode *node_from_index(std::uint32_t index);

    void load_node(ModelNode *parent, const tinygltf::Node &start_node, std::uint32_t scene_index,
                   std::uint32_t node_index);

    void load_materials();

    void load_animations();

    void load_textures();

    void load_skins();

    void load_nodes();

public:
    ModelGpuPbrDataBase(const wrapper::Device &device, const tinygltf::Model &model, std::string name);

    virtual ~ModelGpuPbrDataBase();

    ModelGpuPbrDataBase(const ModelGpuPbrDataBase &) = delete;
    ModelGpuPbrDataBase(ModelGpuPbrDataBase &&) noexcept;

    ModelGpuPbrDataBase &operator=(const ModelGpuPbrDataBase &) = delete;
    ModelGpuPbrDataBase &operator=(ModelGpuPbrDataBase &&) noexcept = default;

    // TODO: Make this const again and move descriptor set allocation to gpu_data_base?
    [[nodiscard]] auto &materials() {
        return m_materials;
    }

    [[nodiscard]] const auto &nodes() const {
        return m_nodes;
    }

    [[nodiscard]] const auto &model() const {
        return m_model;
    }

    [[nodiscard]] const auto &linear_nodes() const {
        return m_linear_nodes;
    }
};

} // namespace inexor::vulkan_renderer::gltf
