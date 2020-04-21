#pragma once

#include <vulkan/vulkan.h>

namespace inexor::vulkan_renderer {

/// @class InexorTextureSampler
struct InexorTextureSampler {
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
};

} // namespace inexor::vulkan_renderer
