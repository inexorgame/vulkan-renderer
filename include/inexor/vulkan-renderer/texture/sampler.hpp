#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::texture {

/// A RAII wrapper class for texture samplers
class Sampler final {
private:
    const wrapper::Device &m_device;
    VkSampler m_sampler;
    std::string m_name;

public:
    /// Create a texture sampler
    /// @param device A const reference to the device wrapper
    ///
    ///
    Sampler(const wrapper::Device &device, VkSamplerCreateInfo sampler_ci, std::string name);

    Sampler(const Sampler &) = delete;
    Sampler(Sampler &&) noexcept;

    ~Sampler();

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }

    Sampler &operator=(const Sampler &) = delete;
    Sampler &operator=(Sampler &&) noexcept = default;
};

} // namespace inexor::vulkan_renderer::texture
