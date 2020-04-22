#pragma once

#include "inexor/vulkan-renderer/texture.hpp"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

///
enum InexorModelMaterialAlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };

///
struct InexorModelMaterial {
    InexorModelMaterialAlphaMode alphaMode = ALPHAMODE_OPAQUE;

    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(1.0f);

    std::shared_ptr<InexorTexture> baseColorTexture;
    std::shared_ptr<InexorTexture> metallicRoughnessTexture;
    std::shared_ptr<InexorTexture> normalTexture;
    std::shared_ptr<InexorTexture> occlusionTexture;
    std::shared_ptr<InexorTexture> emissiveTexture;

    struct TexCoordSets {
        uint8_t baseColor = 0;
        uint8_t metallicRoughness = 0;
        uint8_t specularGlossiness = 0;
        uint8_t normal = 0;
        uint8_t occlusion = 0;
        uint8_t emissive = 0;
    };

    TexCoordSets texCoordSets;

    struct InexorModelExtension {
        std::shared_ptr<InexorTexture> specularGlossinessTexture;
        std::shared_ptr<InexorTexture> diffuseTexture;

        glm::vec4 diffuseFactor = glm::vec4(1.0f);
        glm::vec3 specularFactor = glm::vec3(0.0f);
    };

    InexorModelExtension extension;

    struct PbrWorkflows {
        bool metallicRoughness = true;
        bool specularGlossiness = false;
    };

    PbrWorkflows pbrWorkflows;

    // TODO: Should this be part of InexorTexture?
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

} // namespace inexor::vulkan_renderer::gltf_model
