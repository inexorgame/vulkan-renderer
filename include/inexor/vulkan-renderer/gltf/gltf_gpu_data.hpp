#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_vertex.hpp"
#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
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
    ModelNode *find_node(ModelNode *parent, std::uint32_t index);

    ModelNode *node_from_index(std::uint32_t index);

    void load_node(const tinygltf::Model &model, ModelNode *parent, const tinygltf::Node &start_node,
                   std::uint32_t scene_index, std::uint32_t node_index);

    void load_materials(const tinygltf::Model &model);

    void load_animations(const tinygltf::Model &model);

    void load_textures(const tinygltf::Model &model);

    void load_skins(const tinygltf::Model &model);

    void load_nodes(const tinygltf::Model &model);

    std::string m_name;

    const wrapper::Device &m_device;

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

    std::vector<texture::GpuTexture> m_textures;
    std::vector<TextureSampler> m_texture_samplers;

    VkDescriptorSet m_scene_descriptor_set{VK_NULL_HANDLE};
    VkDescriptorSet m_material_descriptor_set{VK_NULL_HANDLE};

    VkDescriptorSetLayout m_node_descriptor_set_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_material_descriptor_set_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_scene_descriptor_set_layout{VK_NULL_HANDLE};

    ModelMatrices m_scene;
    ModelShaderParams m_shader_values;

    std::unique_ptr<wrapper::UniformBuffer<ModelShaderParams>> m_shader_params;
    std::unique_ptr<wrapper::UniformBuffer<ModelMatrices>> m_scene_matrices;

    std::unique_ptr<texture::GpuTexture> m_empty_texture;

    VkDescriptorImageInfo m_brdf_lut_texture;
    VkDescriptorImageInfo m_enviroment_cube_texture;
    VkDescriptorImageInfo m_irradiance_cube_texture;
    VkDescriptorImageInfo m_prefiltered_cube_texture;

    // TODO: Remove this!
    // In case the model contains textures but no default texture sampler, use this one.
    TextureSampler m_default_texture_sampler{VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                             VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};

    ///
    ///
    ///
    void setup_node_descriptor_sets(const ModelNode &node) {
        if (node.mesh) {
            // TODO: Move arguments into make_info<> template
            VkDescriptorSetAllocateInfo desc_set_ai{};
            desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            desc_set_ai.descriptorPool = m_descriptor_pool->descriptor_pool();
            desc_set_ai.pSetLayouts = &m_node_descriptor_set_layout;
            desc_set_ai.descriptorSetCount = 1;

            vkAllocateDescriptorSets(m_device.device(), &desc_set_ai, &node.mesh->ubo->descriptor_set);

            VkWriteDescriptorSet write_desc_set{};
            write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_desc_set.descriptorCount = 1;
            write_desc_set.dstSet = node.mesh->ubo->descriptor_set;
            write_desc_set.dstBinding = 0;
            write_desc_set.pBufferInfo = &node.mesh->ubo->descriptor;

            vkUpdateDescriptorSets(m_device.device(), 1, &write_desc_set, 0, nullptr);
        }

        for (const auto &child : node.children) {
            setup_node_descriptor_sets(child);
        }
    }

    ///
    ///
    ///
    void setup_rendering_resources(RenderGraph *render_graph) {
        std::uint32_t imageSamplerCount = 0;
        std::uint32_t materialCount = 0;
        std::uint32_t meshCount = 0;

        // Environment samplers (radiance, irradiance, brdf lookup table)
        imageSamplerCount += 3;

        for (const auto &material : m_materials) {
            imageSamplerCount += 5;
            materialCount++;
        }
        for (const auto &node : m_linear_nodes) {
            if (node.mesh) {
                meshCount++;
            }
        }

        const std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount)},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount}};

        m_descriptor_pool =
            std::make_unique<wrapper::DescriptorPool>(render_graph->device_wrapper(), pool_sizes, "gltf pool");

        // Scene (matrices and environment maps)
        {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                 nullptr},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            };

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
            descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
            descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

            vkCreateDescriptorSetLayout(render_graph->device(), &descriptorSetLayoutCI, nullptr,
                                        &m_scene_descriptor_set_layout);

            VkDescriptorSetAllocateInfo desc_set_ai{};
            desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            desc_set_ai.descriptorPool = m_descriptor_pool->descriptor_pool();
            desc_set_ai.pSetLayouts = &m_scene_descriptor_set_layout;
            desc_set_ai.descriptorSetCount = 1;

            vkAllocateDescriptorSets(render_graph->device(), &desc_set_ai, &m_scene_descriptor_set);

            std::array<VkWriteDescriptorSet, 5> write_desc_sets{};

            write_desc_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_desc_sets[0].descriptorCount = 1;
            write_desc_sets[0].dstSet = m_scene_descriptor_set;
            write_desc_sets[0].dstBinding = 0;
            write_desc_sets[0].pBufferInfo = &m_shader_params->descriptor;

            write_desc_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_desc_sets[1].descriptorCount = 1;
            write_desc_sets[1].dstSet = m_scene_descriptor_set;
            write_desc_sets[1].dstBinding = 1;
            write_desc_sets[1].pBufferInfo = &m_scene_matrices->descriptor;

            write_desc_sets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_desc_sets[2].descriptorCount = 1;
            write_desc_sets[2].dstSet = m_scene_descriptor_set;
            write_desc_sets[2].dstBinding = 2;
            write_desc_sets[2].pImageInfo = &m_irradiance_cube_texture;

            write_desc_sets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_desc_sets[3].descriptorCount = 1;
            write_desc_sets[3].dstSet = m_scene_descriptor_set;
            write_desc_sets[3].dstBinding = 3;
            write_desc_sets[3].pImageInfo = &m_prefiltered_cube_texture;

            write_desc_sets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_sets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_desc_sets[4].descriptorCount = 1;
            write_desc_sets[4].dstSet = m_scene_descriptor_set;
            write_desc_sets[4].dstBinding = 4;
            write_desc_sets[4].pImageInfo = &m_brdf_lut_texture;

            vkUpdateDescriptorSets(m_device.device(), static_cast<uint32_t>(write_desc_sets.size()),
                                   write_desc_sets.data(), 0, nullptr);
        }

        // Material (samplers)
        {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            };

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
            descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
            descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

            vkCreateDescriptorSetLayout(render_graph->device(), &descriptorSetLayoutCI, nullptr,
                                        &m_material_descriptor_set_layout);

            // Per-material descriptor sets
            for (const auto &material : m_materials) {

                VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
                descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptorSetAllocInfo.descriptorPool = m_descriptor_pool->descriptor_pool();
                descriptorSetAllocInfo.pSetLayouts = &m_material_descriptor_set_layout;
                descriptorSetAllocInfo.descriptorSetCount = 1;

                vkAllocateDescriptorSets(render_graph->device(), &descriptorSetAllocInfo, &m_material_descriptor_set);

                std::vector<VkDescriptorImageInfo> imageDescriptors = {
                    m_empty_texture->descriptor(), m_empty_texture->descriptor(),
                    material.normal_texture ? material.normal_texture->descriptor() : m_empty_texture->descriptor(),
                    material.occlusion_texture ? material.occlusion_texture->descriptor()
                                               : m_empty_texture->descriptor(),
                    material.emissive_texture ? material.emissive_texture->descriptor()
                                              : m_empty_texture->descriptor()};

                // TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is
                // present

                if (material.metallic_roughness) {
                    if (material.base_color_texture) {
                        imageDescriptors[0] = material.base_color_texture->descriptor();
                    }
                    if (material.metallic_roughness_texture) {
                        imageDescriptors[1] = material.metallic_roughness_texture->descriptor();
                    }
                }

                if (material.specular_glossiness) {
                    if (material.extension.diffuse_texture) {
                        imageDescriptors[0] = material.extension.diffuse_texture->descriptor();
                    }
                    if (material.extension.specular_glossiness_texture) {
                        imageDescriptors[1] = material.extension.specular_glossiness_texture->descriptor();
                    }
                }

                std::array<VkWriteDescriptorSet, 5> write_desc_set{};

                for (size_t i = 0; i < imageDescriptors.size(); i++) {
                    write_desc_set[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write_desc_set[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write_desc_set[i].descriptorCount = 1;
                    write_desc_set[i].dstSet = m_material_descriptor_set;
                    write_desc_set[i].dstBinding = static_cast<uint32_t>(i);
                    write_desc_set[i].pImageInfo = &imageDescriptors[i];
                }

                vkUpdateDescriptorSets(m_device.device(), static_cast<uint32_t>(write_desc_set.size()),
                                       write_desc_set.data(), 0, nullptr);
            }

            // Model node (matrices)
            {
                std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                };

                // TODO: Move into make_info<> template!
                VkDescriptorSetLayoutCreateInfo desc_set_layout_ci{};
                desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                desc_set_layout_ci.pBindings = setLayoutBindings.data();
                desc_set_layout_ci.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

                vkCreateDescriptorSetLayout(render_graph->device(), &desc_set_layout_ci, nullptr,
                                            &m_node_descriptor_set_layout);

                // Per-node descriptor set
                for (auto &node : m_nodes) {
                    setup_node_descriptor_sets(node);
                }
            }
        }
    }

protected:
    std::vector<ModelVertex> m_vertices{};
    std::vector<std::uint32_t> m_indices{};

public:
    VkDescriptorSet scene_descriptor_set{VK_NULL_HANDLE};

    ModelGpuData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data, VkDescriptorImageInfo brdf_lut_texture,
                 VkDescriptorImageInfo enviroment_cube_texture, VkDescriptorImageInfo irradiance_cube_texture,
                 VkDescriptorImageInfo prefiltered_cube_texture, const glm::mat4 &model_matrix,
                 const glm::mat4 &proj_matrix);

    ModelGpuData(const ModelGpuData &) = delete;
    ModelGpuData(ModelGpuData &&) noexcept;

    ~ModelGpuData();

    ModelGpuData &operator=(const ModelGpuData &) = delete;
    ModelGpuData &operator=(ModelGpuData &&) noexcept = default;

    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const auto &indices() const {
        return m_indices;
    };

    [[nodiscard]] std::size_t vertex_count() const {
        return m_vertices.size();
    }

    [[nodiscard]] std::size_t index_count() const {
        return m_indices.size();
    }

    [[nodiscard]] const auto &nodes() const {
        return m_nodes;
    }
};

} // namespace inexor::vulkan_renderer::gltf
