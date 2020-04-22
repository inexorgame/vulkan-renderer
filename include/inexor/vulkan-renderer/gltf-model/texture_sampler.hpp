#pragma once

#include <vulkan/vulkan.h>

namespace inexor::vulkan_renderer::gltf_model {

/// @class InexorTextureSampler
struct InexorTextureSampler {
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
};

} // namespace inexor::vulkan_renderer::gltf_model
