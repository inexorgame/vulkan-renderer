#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <memory>

namespace inexor::vulkan_renderer::pbr {

/// @brief A wrapper class for bidirectional reflectance distribution (BRDF) look up table (LUT) generator.
class BRDFLUTGenerator {
private:
    const wrapper::Device &m_device;
    std::unique_ptr<wrapper::Image> m_brdf_lut_image;
    std::unique_ptr<wrapper::Framebuffer> m_framebuffer;

    VkPipelineLayout m_pipeline_layout;
    VkRenderPass m_renderpass;
    VkPipeline m_pipeline;
    VkDescriptorSetLayout m_desc_set_layout;

public:
    /// @brief Default constructor.
    /// @param device The Vulkan device
    BRDFLUTGenerator(const wrapper::Device &device);

    ~BRDFLUTGenerator();

    [[nodiscard]] const auto &descriptor() const {
        return m_brdf_lut_image->descriptor();
    }
};

} // namespace inexor::vulkan_renderer::pbr
