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
    // TODO: Should we change this so it returns the texture? I think so.

    /// Default constructor
    /// @param device A const reference to the Vulkan device wrapper
    BRDFLUTGenerator(wrapper::Device &device);

    [[nodiscard]] VkDescriptorImageInfo descriptor_image_info() const {
        return m_brdf_texture->descriptor_image_info;
    }
};

} // namespace inexor::vulkan_renderer::pbr
