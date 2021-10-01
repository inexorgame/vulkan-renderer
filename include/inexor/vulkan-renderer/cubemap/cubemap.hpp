#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include <memory>

namespace inexor::vulkan_renderer::cubemap {

/// @brief A wrapper class for cubemaps
class Cubemap {

private:
    std::unique_ptr<wrapper::Image> m_cubemap_image;
    std::unique_ptr<wrapper::OffscreenFramebuffer> m_offscreen_framebuffer;
    VkSampler m_sampler{VK_NULL_HANDLE};

public:
    ///
    ///
    Cubemap(const wrapper::Device &device);
};

} // namespace inexor::vulkan_renderer::cubemap
