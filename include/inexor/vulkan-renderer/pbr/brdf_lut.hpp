#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <memory>

namespace inexor::vulkan_renderer {

// TODO: Package this into a separate task using taskflow!

/// @brief A wrapper class for bidirectional reflectance distribution (BRDF) look up table (LUT) generator.
class BrdfLutGenerator {
private:
    std::unique_ptr<wrapper::Image> m_brdf_lut_image;

public:
    /// @brief Default constructor.
    /// @param device The Vulkan device
    /// @param shaders The vertex and fragment shader of the brdf generator
    BrdfLutGenerator(VkDevice device, const std::array<wrapper::Shader, 2> &shaders);

    ~BrdfLutGenerator();
};

} // namespace inexor::vulkan_renderer
