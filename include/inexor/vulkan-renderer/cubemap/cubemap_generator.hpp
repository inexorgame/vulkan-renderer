#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include <memory>

namespace inexor::vulkan_renderer::cubemap {

class CubemapGenerator {
private:
    std::unique_ptr<GpuCubemap> m_cubemap_texture;
    std::unique_ptr<wrapper::OffscreenFramebuffer> m_offscreen_framebuffer;
    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    void setup_pipeline();
    void render_cubemap();

public:
    CubemapGenerator(const wrapper::Device &device);

    // TODO: Add get methods for cubemap!
};

} // namespace inexor::vulkan_renderer::cubemap
