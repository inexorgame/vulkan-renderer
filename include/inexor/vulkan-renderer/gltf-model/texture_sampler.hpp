#pragma once

#include <vulkan/vulkan.h>

namespace inexor::vulkan_renderer::gltf_model {

struct TextureSampler {
    VkFilter mag_filter;
    VkFilter min_filter;
    VkSamplerAddressMode address_mode_u;
    VkSamplerAddressMode address_mode_v;
    VkSamplerAddressMode address_mode_w;
};

} // namespace inexor::vulkan_renderer::gltf_model
