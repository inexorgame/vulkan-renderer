#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <memory>

namespace inexor::vulkan_renderer::pbr {

// TODO: Package this into a separate task using taskflow for parallelization!

/// @brief A wrapper class for bidirectional reflectance distribution (BRDF) look up table (LUT) generator.
class BrdfLutGenerator {
private:
    std::unique_ptr<wrapper::Image> m_brdf_lut_image;
    std::unique_ptr<wrapper::Framebuffer> m_framebuffer;

public:
    /// @brief Default constructor.
    /// @param device The Vulkan device
    BrdfLutGenerator(const wrapper::Device &device);
};

} // namespace inexor::vulkan_renderer::pbr
