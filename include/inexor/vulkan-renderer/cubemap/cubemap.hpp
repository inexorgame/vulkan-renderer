#pragma once

#include "inexor/vulkan-renderer/cubemap/texture2d.hpp"
#include "inexor/vulkan-renderer/cubemap/texture_cubemap.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include <memory>

namespace inexor::vulkan_renderer::cubemap {

/// @brief A wrapper class for cubemaps
class Cubemap {

private:
    std::unique_ptr<wrapper::GpuTexture> m_cubemap_texture;
    std::unique_ptr<wrapper::OffscreenFramebuffer> m_offscreen_framebuffer;
    VkSampler m_sampler{VK_NULL_HANDLE};

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    /* struct CubemapTextures {
        TextureCubeMap environment;
        Texture2D empty;
        Texture2D brdf_lut;
        TextureCubeMap irradiance;
        TextureCubeMap prefiltered;
    };

    CubemapTextures m_textures;
    */

public:
    /// @brief
    /// @param device
    Cubemap(const wrapper::Device &device);

    [[nodiscard]] auto &image_wrapper() const {
        return m_cubemap_texture->image_wrapper();
    }
};

} // namespace inexor::vulkan_renderer::cubemap
