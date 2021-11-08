#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::cubemap {

GpuCubemap::GpuCubemap(const wrapper::Device &device, const VkFormat format, const std::uint32_t dim,
                       const std::uint32_t mip_levels, const std::string name)
    : TextureBase(device) {

    assert(device.device());
    assert(dim > 0);
    assert(mip_levels > 0);
    assert(!name.empty());

    m_attributes.name = name;
    m_attributes.width = dim;
    m_attributes.height = dim;

    auto image_ci = wrapper::make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format;
    image_ci.extent.width = dim;
    image_ci.extent.height = dim;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = mip_levels;
    image_ci.arrayLayers = FACE_COUNT;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>();
    image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    image_view_ci.format = format;
    image_view_ci.subresourceRange = {};
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.levelCount = mip_levels;
    image_view_ci.subresourceRange.layerCount = FACE_COUNT;

    m_image = std::make_unique<wrapper::Image>(m_device, image_ci, image_view_ci, name);

    auto sampler_ci = wrapper::make_info<VkSamplerCreateInfo>();
    sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = static_cast<float>(mip_levels);
    // TODO: That means cubemaps don't support anisotropic filtering yet!
    sampler_ci.maxAnisotropy = 1.0f;
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    create_texture_sampler(sampler_ci);

    wrapper::OnceCommandBuffer single_command(device);
    single_command.create_command_buffer();
    single_command.start_recording();
    {
        // Prepare for copy operation: change the image layout for all cubemap faces to transfer destination
        m_image->transition_image_layout(single_command.command_buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         mip_levels, FACE_COUNT);
    }
    single_command.end_recording_and_submit_command();
}

void GpuCubemap::copy_from_image(const VkCommandBuffer cmd_buf, const VkImage source_image, const std::uint32_t face,
                                 const std::uint32_t mip_level, const std::uint32_t width, const std::uint32_t height) {
    VkImageCopy region{};

    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset = {0, 0, 0};

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.baseArrayLayer = face;
    region.dstSubresource.mipLevel = mip_level;
    region.dstSubresource.layerCount = 1;
    region.dstOffset = {0, 0, 0};

    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(cmd_buf, source_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image->image(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

} // namespace inexor::vulkan_renderer::cubemap
