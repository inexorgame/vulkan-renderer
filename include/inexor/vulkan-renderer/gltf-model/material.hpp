#pragma once

#include "inexor/vulkan-renderer/texture.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <memory>

namespace inexor::vulkan_renderer::gltf_model {

#undef OPAQUE // defined by wingdi.h, which is included by windows.h under MINGW
enum class AlphaMode { OPAQUE, MASK, BLEND };

struct Material {
    AlphaMode alpha_mode = AlphaMode::OPAQUE;

    float alpha_cutoff = 1.0f;
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;

    glm::vec4 base_color_factor = glm::vec4(1.0f);
    glm::vec4 emissive_factor = glm::vec4(1.0f);

    std::shared_ptr<Texture> base_color_texture;
    std::shared_ptr<Texture> metallic_roughness_texture;
    std::shared_ptr<Texture> normal_texture;
    std::shared_ptr<Texture> occlusion_texture;
    std::shared_ptr<Texture> emissive_texture;

    struct TexCoordSets {
        std::uint8_t base_color = 0;
        std::uint8_t metallic_roughness = 0;
        std::uint8_t specular_glossiness = 0;
        std::uint8_t normal = 0;
        std::uint8_t occlusion = 0;
        std::uint8_t emissive = 0;
    };

    TexCoordSets tex_coord_sets;

    struct ModelExtension {
        std::shared_ptr<Texture> specular_glossiness_texture;
        std::shared_ptr<Texture> diffuse_texture;

        glm::vec4 diffuse_factor = glm::vec4(1.0f);
        glm::vec3 specular_factor = glm::vec3(0.0f);
    };

    ModelExtension extension;

    struct PbrWorkflow {
        bool metallic_roughness = true;
        bool specular_glossiness = false;
    };

    PbrWorkflow pbr_workflow;

    // TODO: Should this be part of Texture?
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

} // namespace inexor::vulkan_renderer::gltf_model
