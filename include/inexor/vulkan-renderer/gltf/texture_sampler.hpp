#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace inexor::vulkan_renderer::gltf {

/// @brief A wrapper for texture samplers for rendering glTF2 models.
class TextureSampler {
private:
    VkFilter m_min_filter;
    VkFilter m_mag_filter;
    VkSamplerAddressMode m_address_mode_u;
    VkSamplerAddressMode m_address_mode_v;
    VkSamplerAddressMode m_address_mode_w;

public:
    TextureSampler() = default;

    /// @brief
    /// @param filter_mag
    /// @param filter_min
    /// @param mode_u
    /// @param mode_v
    /// @param mode_w
    TextureSampler(const VkFilter filter_mag, const VkFilter filter_min, const VkSamplerAddressMode mode_u,
                   const VkSamplerAddressMode mode_v, const VkSamplerAddressMode mode_w);

    /// @brief Overloaded constructor.
    /// @param filter_min
    /// @param filter_mag
    /// @param mode_s
    /// @param mode_t
    /// @param mode_t
    TextureSampler(const std::uint32_t filter_min, const std::uint32_t filter_mag, const std::uint32_t mode_s,
                   const std::uint32_t mode_t);

    [[nodiscard]] VkFilter min_filter() const {
        return m_min_filter;
    }

    [[nodiscard]] VkFilter mag_filter() const {
        return m_mag_filter;
    }

    [[nodiscard]] VkSamplerAddressMode address_mode_u() const {
        return m_address_mode_u;
    }

    [[nodiscard]] VkSamplerAddressMode address_mode_v() const {
        return m_address_mode_v;
    }

    [[nodiscard]] VkSamplerAddressMode address_mode_w() const {
        return m_address_mode_w;
    }
};

} // namespace inexor::vulkan_renderer::gltf
