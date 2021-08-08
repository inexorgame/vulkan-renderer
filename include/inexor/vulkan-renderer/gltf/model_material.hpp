#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model materials.
struct ModelMaterial {
    enum class AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
    AlphaMode alphaMode = AlphaMode::ALPHAMODE_OPAQUE;

    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(1.0f);

    void *baseColorTexture;
    void *metallicRoughnessTexture;
    void *normalTexture;
    void *occlusionTexture;
    void *emissiveTexture;

    struct TexCoordSets {
        uint8_t baseColor = 0;
        uint8_t metallicRoughness = 0;
        uint8_t specularGlossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    } texCoordSets;

    struct Extension {
        void *specularGlossinessTexture;
        void *diffuseTexture;
        glm::vec4 diffuseFactor = glm::vec4(1.0f);
        glm::vec3 specularFactor = glm::vec3(0.0f);
    } extension;

    struct PbrWorkflows {
        bool metallicRoughness = true;
        bool specularGlossiness = false;
    } pbrWorkflows;
};

} // namespace inexor::vulkan_renderer::gltf
