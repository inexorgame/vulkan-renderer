#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include <memory>

namespace inexor::vulkan_renderer::cubemap {

// TODO: Abstract this so any stuff can be rendered into a cubemap?
// TODO: How to integrate this into rendergraph?
class CubemapGenerator {
private:
    std::unique_ptr<wrapper::OffscreenFramebuffer> m_offscreen_framebuffer;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<GpuCubemap> m_cubemap_texture;
    std::unique_ptr<GpuCubemap> m_irradiance_cube_texture;
    std::unique_ptr<GpuCubemap> m_prefiltered_cube_texture;

    // TODO: Implement!
    void setup_pipeline();
    void render_cubemap();

    static constexpr std::uint32_t CUBE_FACE_COUNT{6};

    void draw_node(const wrapper::CommandBuffer &cmd_buf, const gltf::ModelNode &node);

public:
    CubemapGenerator(const wrapper::Device &device, const skybox::SkyboxGpuData &skybox);

    [[nodiscard]] VkDescriptorImageInfo cubemap_descriptor_image_info() const {
        return m_cubemap_texture->descriptor_image_info;
    }

    [[nodiscard]] VkDescriptorImageInfo irradiance_descriptor_image_info() const {
        return m_irradiance_cube_texture->descriptor_image_info;
    }

    [[nodiscard]] VkDescriptorImageInfo prefiltered_descriptor_image_info() const {
        return m_prefiltered_cube_texture->descriptor_image_info;
    }
};

} // namespace inexor::vulkan_renderer::cubemap
