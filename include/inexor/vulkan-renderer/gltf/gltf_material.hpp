#pragma once

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

    // TODO: Change types!
    void *base_color_texture;
    void *metallic_roughness_texture;
    void *normal_texture;
    void *occlusion_texture;
    void *emissive_texture;

    struct TexCoordSets {
        uint8_t base_color = 0;
        uint8_t metallic_roughness = 0;
        uint8_t specular_glossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    } texture_coordinate_set;

    struct Extension {
        void *specular_glossiness_texture;
        void *diffuse_texture;
        glm::vec4 diffuse_factor = glm::vec4(1.0f);
        glm::vec3 specular_factor = glm::vec3(0.0f);
    } extension;

    struct PbrWorkflows {
        bool metallic_roughness = true;
        bool specular_glossiness = false;
    } pbr_workflows;
};

} // namespace inexor::vulkan_renderer::gltf
