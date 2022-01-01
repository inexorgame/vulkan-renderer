#pragma once

#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inexor::vulkan_renderer::gltf {

enum class PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };

enum class AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };

struct TextureCoordinateSet {
    std::uint8_t base_color{0};
    std::uint8_t metallic_roughness{0};
    std::uint8_t specular_glossiness{0};
    std::uint8_t normal{0};
    std::uint8_t occlusion{0};
    std::uint8_t emissive{0};
};

struct Extension {
    texture::GpuTexture *specular_glossiness_texture{nullptr};
    texture::GpuTexture *diffuse_texture{nullptr};
    glm::vec4 diffuse_factor = glm::vec4(1.0f);
    glm::vec3 specular_factor = glm::vec3(0.0f);
};

/// A struct for glTF2 model materials
struct ModelMaterial {

    AlphaMode alpha_mode = AlphaMode::ALPHAMODE_OPAQUE;

    float alpha_cutoff{1.0f};
    float metallic_factor{1.0f};
    float roughness_factor{1.0f};

    glm::vec4 base_color_factor = glm::vec4(1.0f);
    glm::vec4 emissive_factor = glm::vec4(1.0f);

    // TODO: Make this const?
    texture::GpuTexture *base_color_texture{nullptr};
    texture::GpuTexture *metallic_roughness_texture{nullptr};
    texture::GpuTexture *normal_texture{nullptr};
    texture::GpuTexture *occlusion_texture{nullptr};
    texture::GpuTexture *emissive_texture{nullptr};

    TextureCoordinateSet texture_coordinate_set;

    Extension extension;

    bool metallic_roughness{true};
    bool specular_glossiness{false};

    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};
};

///
struct MaterialPushConstBlock {
    glm::vec4 baseColorFactor;
    glm::vec4 emissiveFactor;
    glm::vec4 diffuseFactor;
    glm::vec4 specularFactor;

    std::int32_t colorTextureSet{-1};
    std::int32_t PhysicalDescriptorTextureSet{-1};
    std::int32_t normalTextureSet{-1};
    std::int32_t occlusionTextureSet{-1};
    std::int32_t emissiveTextureSet{-1};

    float metallicFactor{0.0f};
    float roughnessFactor{0.0f};
    float alphaMask{0.0f};
    float alphaMaskCutoff{0.0f};
    float workflow;

    ///
    ///
    ///
    MaterialPushConstBlock(const ModelMaterial &material) {
        emissiveFactor = material.emissive_factor;
        alphaMaskCutoff = material.alpha_cutoff;
        alphaMask = static_cast<float>(material.alpha_mode == AlphaMode::ALPHAMODE_MASK);

        if (material.base_color_texture != nullptr) {
            colorTextureSet = material.texture_coordinate_set.base_color;
        }
        if (material.normal_texture != nullptr) {
            normalTextureSet = material.texture_coordinate_set.normal;
        }
        if (material.occlusion_texture != nullptr) {
            emissiveTextureSet = material.texture_coordinate_set.occlusion;
        }
        if (material.emissive_texture != nullptr) {
            emissiveTextureSet = material.texture_coordinate_set.emissive;
        }
        if (material.emissive_texture != nullptr) {
            emissiveTextureSet = material.texture_coordinate_set.emissive;
        }

        // Metallic roughness workflow
        if (material.metallic_roughness) {
            workflow = static_cast<float>(PBRWorkflows::PBR_WORKFLOW_METALLIC_ROUGHNESS);
            baseColorFactor = material.base_color_factor;
            metallicFactor = material.metallic_factor;
            roughnessFactor = material.roughness_factor;

            if (material.metallic_roughness_texture != nullptr) {
                PhysicalDescriptorTextureSet = material.texture_coordinate_set.metallic_roughness;
            }
            if (material.base_color_texture != nullptr) {
                colorTextureSet = material.texture_coordinate_set.base_color;
            }
        }

        // Specular glossiness workflow
        if (material.specular_glossiness) {
            workflow = static_cast<float>(PBRWorkflows::PBR_WORKFLOW_SPECULAR_GLOSINESS);

            if (material.extension.specular_glossiness_texture != nullptr) {
                PhysicalDescriptorTextureSet = material.texture_coordinate_set.specular_glossiness;
            }
            if (material.extension.diffuse_texture != nullptr) {
                colorTextureSet = material.texture_coordinate_set.base_color;
            }

            diffuseFactor = material.extension.diffuse_factor;
            specularFactor = glm::vec4(material.extension.specular_factor, 1.0f);
        }
    }
};

} // namespace inexor::vulkan_renderer::gltf
