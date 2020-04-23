#pragma once

#include "inexor/vulkan-renderer/texture.hpp"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

///
enum class AlphaMode { opaque, mask, blend };

///
struct Material {
    AlphaMode alphaMode = AlphaMode::opaque;

    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(1.0f);

    std::shared_ptr<Texture> baseColorTexture;
    std::shared_ptr<Texture> metallicRoughnessTexture;
    std::shared_ptr<Texture> normalTexture;
    std::shared_ptr<Texture> occlusionTexture;
    std::shared_ptr<Texture> emissiveTexture;

    struct TexCoordSets {
        uint8_t baseColor = 0;
        uint8_t metallicRoughness = 0;
        uint8_t specularGlossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    };

    TexCoordSets texCoordSets;

    struct ModelExtension {
        std::shared_ptr<Texture> specularGlossinessTexture;
        std::shared_ptr<Texture> diffuseTexture;

        glm::vec4 diffuseFactor = glm::vec4(1.0f);
        glm::vec3 specularFactor = glm::vec3(0.0f);
    };

    ModelExtension extension;

    struct PbrWorkflows {
        bool metallicRoughness = true;
        bool specularGlossiness = false;
    };

    PbrWorkflows pbrWorkflows;

    // TODO: Should this be part of Texture?
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

} // namespace inexor::vulkan_renderer::gltf_model
