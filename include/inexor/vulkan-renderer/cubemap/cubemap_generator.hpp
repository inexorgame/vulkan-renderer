#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include <memory>

namespace inexor::vulkan_renderer::cubemap {

// TODO: Abstract this so any stuff can be rendered into a cubemap?
class CubemapGenerator {
private:
    std::unique_ptr<GpuCubemap> m_cubemap_texture;
    std::unique_ptr<wrapper::OffscreenFramebuffer> m_offscreen_framebuffer;
    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    // TODO: Implement!
    void setup_pipeline();
    void render_cubemap();

public:
    CubemapGenerator(const wrapper::Device &device);

    [[nodiscard]] const auto &image_wrapper() const {
        return m_cubemap_texture->image_wrapper();
    }

    [[nodiscard]] VkDescriptorImageInfo descriptor() const {
        return m_cubemap_texture->descriptor();
    }
};

} // namespace inexor::vulkan_renderer::cubemap
