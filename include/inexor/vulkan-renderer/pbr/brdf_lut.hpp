#pragma once

#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <memory>

namespace inexor::vulkan_renderer::pbr {

/// A wrapper class for bidirectional reflectance distribution (BRDF) look up table (LUT) generator
class BRDFLUTGenerator {
private:
    const wrapper::Device &m_device;
    std::unique_ptr<texture::GpuTexture> m_brdf_texture;
    std::unique_ptr<wrapper::Framebuffer> m_framebuffer;

public:
    /// Default constructor
    /// @param device A const reference to the Vulkan device wrapper
    BRDFLUTGenerator(const wrapper::Device &device);

    [[nodiscard]] VkDescriptorImageInfo descriptor() {
        return m_brdf_texture->descriptor();
    }
};

} // namespace inexor::vulkan_renderer::pbr
