#pragma once

#include <vulkan/vulkan_core.h>

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
    /// @param sampler_ci The sampler create info
    /// @param name The internal debug name of the sampler
    Sampler(const Device &device, const VkSamplerCreateInfo &sampler_ci, std::string name);

    /// Overloaded constructor which creates a default sampler
    /// @param device The device wrapper
    /// @param name The internal debug name of the sampler
    Sampler(const Device &device, std::string name);
    ~Sampler();

    Sampler(const Sampler &) = delete;
    Sampler(Sampler &&) noexcept;

    Sampler &operator=(const Sampler &) = delete;
    Sampler &operator=(Sampler &&) = delete;

    [[nodiscard]] VkSampler sampler() const noexcept {
        return m_sampler;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
