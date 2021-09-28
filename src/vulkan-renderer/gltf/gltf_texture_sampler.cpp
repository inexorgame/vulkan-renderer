#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"

namespace inexor::vulkan_renderer::gltf {

TextureSampler::TextureSampler(const VkFilter magfilter, const VkFilter minfilter, const VkSamplerAddressMode mode_u,
                               const VkSamplerAddressMode mode_v, const VkSamplerAddressMode mode_w)

    : m_mag_filter(magfilter), m_min_filter(minfilter), m_address_mode_u(mode_u), m_address_mode_v(mode_v),
      m_address_mode_w(mode_w) {}

TextureSampler::TextureSampler(const std::uint32_t filter_min, const std::uint32_t filter_mag,
                               const std::uint32_t mode_s, const std::uint32_t mode_t) {

    /// @brief Convert the filtermode into a VkFilterMode.
    /// @param filter_mode
    auto get_filter_mode = [](const std::uint32_t filter_mode) {
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

    /// @brief Convert the wrap mode into a VkSamplerAddressMode.
    /// @param wrap_mode
    auto get_wrap_mode = [](const std::uint32_t wrap_mode) {
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

    m_min_filter = get_filter_mode(filter_min);
    m_mag_filter = get_filter_mode(filter_mag);

    m_address_mode_u = get_wrap_mode(mode_s);
    m_address_mode_v = get_wrap_mode(mode_t);
    m_address_mode_w = m_address_mode_v;
}

} // namespace inexor::vulkan_renderer::gltf
