#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

VkSamplerCreateInfo make_sampler_ci(const VkFilter mag_filter, const VkFilter min_filter,
                                    const VkSamplerAddressMode address_mode_u,
                                    const VkSamplerAddressMode address_mode_v,
                                    const VkSamplerAddressMode address_mode_w, const std::uint32_t miplevel_count) {
    assert(miplevel_count > 0);

    VkSamplerCreateInfo sampler_ci{};
    sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_ci.magFilter = mag_filter;
    sampler_ci.minFilter = min_filter;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.addressModeU = address_mode_u;
    sampler_ci.addressModeV = address_mode_v;
    sampler_ci.addressModeW = address_mode_w;
    sampler_ci.compareOp = VK_COMPARE_OP_NEVER;
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_ci.maxAnisotropy = 1.0;
    sampler_ci.anisotropyEnable = VK_FALSE;
    sampler_ci.maxLod = static_cast<float>(miplevel_count);

    // TODO: Check if anisotropic filter is even on
    // TODO: Why 8.0f
    sampler_ci.maxAnisotropy = 8.0f;
    sampler_ci.anisotropyEnable = VK_TRUE;
    return sampler_ci;
}

VkSamplerCreateInfo make_sampler_ci(const std::uint32_t filter_min, const std::uint32_t filter_mag,
                                    const std::uint32_t mode_s, const std::uint32_t mode_t,
                                    const std::uint32_t miplevel_count) {

    const auto min_filter = get_filter_mode(filter_min);
    const auto mag_filter = get_filter_mode(filter_mag);
    const auto address_mode_u = get_wrap_mode(mode_s);
    const auto address_mode_v = get_wrap_mode(mode_t);
    const auto address_mode_w = address_mode_v;

    return make_sampler_ci(min_filter, mag_filter, address_mode_u, address_mode_v, address_mode_w, miplevel_count);
}

VkSamplerCreateInfo make_sampler_ci(TextureSampler sampler) {
    return make_sampler_ci(sampler.min_filter, sampler.mag_filter, sampler.address_mode_u, sampler.address_mode_v,
                           sampler.address_mode_w);
}

VkSamplerCreateInfo make_sampler_ci(const std::uint32_t miplevel_count) {
    return make_sampler_ci(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                           VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, miplevel_count);
}

} // namespace inexor::vulkan_renderer::gltf
