#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model materials.
struct ModelMaterial {

    enum class AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
    AlphaMode alpha_mode = AlphaMode::ALPHAMODE_OPAQUE;

    float alpha_cutoff = 1.0f;
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;

    glm::vec4 base_color_factor = glm::vec4(1.0f);
    glm::vec4 emissive_factor = glm::vec4(1.0f);

    // TODO: Make this const?
    wrapper::GpuTexture *base_color_texture;
    wrapper::GpuTexture *metallic_roughness_texture;
    wrapper::GpuTexture *normal_texture;
    wrapper::GpuTexture *occlusion_texture;
    wrapper::GpuTexture *emissive_texture;

    struct TexCoordSets {
        std::uint8_t base_color = 0;
        std::uint8_t metallic_roughness = 0;
        std::uint8_t specular_glossiness = 0;
        std::uint8_t normal = 0;
        std::uint8_t occlusion = 0;
        std::uint8_t emissive = 0;
    } texture_coordinate_set;

    struct Extension {
        wrapper::GpuTexture *specular_glossiness_texture;
        wrapper::GpuTexture *diffuse_texture;
        glm::vec4 diffuse_factor = glm::vec4(1.0f);
        glm::vec3 specular_factor = glm::vec3(0.0f);
    } extension;

    struct PBRWorkflows {
        bool metallic_roughness = true;
        bool specular_glossiness = false;
    } pbr_workflows;

    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

} // namespace inexor::vulkan_renderer::gltf
