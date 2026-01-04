#pragma once

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for VkSampler
class Sampler {
private:
    const Device &m_device;
    VkSampler m_sampler{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the sampler
    /// @param sampler_ci The sampler create info
    Sampler(const Device &device, std::string name,
            const VkSamplerCreateInfo &sampler_ci = make_info<VkSamplerCreateInfo>({
                // NOTE: These are the default sampler settings
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_FALSE,
                .maxAnisotropy = 1.0f,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = 0.0f,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
            }));

    Sampler(const Sampler &) = delete;
    Sampler(Sampler &&) noexcept;
    ~Sampler();

    Sampler &operator=(const Sampler &) = delete;
    Sampler &operator=(Sampler &&) = delete;

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
