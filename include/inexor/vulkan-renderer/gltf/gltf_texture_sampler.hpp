#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace inexor::vulkan_renderer::gltf {

const auto get_filter_mode = [](const std::uint32_t filter_mode) {
    switch (filter_mode) {
    case 9728:
    case 9985:
    case 9984:
        return VK_FILTER_NEAREST;
    case 9729:
    case 9986:
    case 9987:
        return VK_FILTER_LINEAR;
    }
    return VK_FILTER_NEAREST;
};

const auto get_wrap_mode = [](const std::uint32_t wrap_mode) {
    switch (wrap_mode) {
    case 10497:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case 33071:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case 33648:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

struct TextureSampler {

    TextureSampler() = default;

    TextureSampler(const std::uint32_t filter_min, const std::uint32_t filter_mag, const std::uint32_t mode_s,
                   const std::uint32_t mode_t) {
        min_filter = get_filter_mode(filter_min);
        mag_filter = get_filter_mode(filter_mag);
        address_mode_u = get_wrap_mode(mode_s);
        address_mode_v = get_wrap_mode(mode_t);
        address_mode_w = address_mode_v;
    }

    VkFilter mag_filter{VK_FILTER_LINEAR};
    VkFilter min_filter{VK_FILTER_LINEAR};
    VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
    VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
    VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
};

[[nodiscard]] VkSamplerCreateInfo make_sampler_ci(VkFilter min_filter, VkFilter mag_filter,
                                                  VkSamplerAddressMode address_mode_u,
                                                  VkSamplerAddressMode address_mode_v,
                                                  VkSamplerAddressMode address_mode_w, std::uint32_t miplevel_count);

[[nodiscard]] VkSamplerCreateInfo make_sampler_ci(TextureSampler sampler);

[[nodiscard]] VkSamplerCreateInfo make_sampler_ci(std::uint32_t miplevel_count);

} // namespace inexor::vulkan_renderer::gltf
