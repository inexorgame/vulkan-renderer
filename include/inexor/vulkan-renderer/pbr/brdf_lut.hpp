#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <memory>

namespace inexor::vulkan_renderer::pbr {

/// A wrapper class for bidirectional reflectance distribution (BRDF) look up table (LUT) generator
class BRDFLUTGenerator {
private:
    const wrapper::Device &m_device;
    std::unique_ptr<wrapper::Image> m_brdf_lut_image;
    std::unique_ptr<wrapper::Framebuffer> m_framebuffer;

public:
    /// Default constructor
    /// @param device A const reference to the Vulkan device wrapper
    BRDFLUTGenerator(const wrapper::Device &device);

    [[nodiscard]] const auto &descriptor() const {
        return m_brdf_lut_image->descriptor();
    }
};

} // namespace inexor::vulkan_renderer::pbr
