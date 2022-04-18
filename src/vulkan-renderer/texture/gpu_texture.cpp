#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <utility>

namespace inexor::vulkan_renderer::texture {

VkImageCreateInfo GpuTexture::fill_image_ci(const VkFormat format, const std::uint32_t width,
                                            const std::uint32_t height) {
    auto image_ci = wrapper::make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = width;
    image_ci.extent.height = height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);
    image_ci.arrayLayers = 1;
    image_ci.format = format;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return image_ci;
}

VkImageViewCreateInfo GpuTexture::fill_image_view_ci(const VkFormat format) {
    auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;
    return image_view_ci;
}

VkSamplerCreateInfo GpuTexture::fill_sampler_ci(const wrapper::Device &device) {
    auto sampler_ci = wrapper::make_info<VkSamplerCreateInfo>();
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.compareEnable = VK_FALSE;
    sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    // Check if anisotropic filtering is available before we enable its use

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device.physical_device(), &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(device.physical_device(), &graphics_card_properties);

    if (device_features.samplerAnisotropy) {
        sampler_ci.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_ci.anisotropyEnable = VK_TRUE;
    } else {
        sampler_ci.maxAnisotropy = 1.0;
        sampler_ci.anisotropyEnable = VK_FALSE;
    }

    return sampler_ci;
}

GpuTexture::GpuTexture(const wrapper::Device &device, const void *texture_data, const std::size_t texture_size,
                       const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
                       const VkSamplerCreateInfo sampler_ci, const std::string name)
    : GpuTexture(device, image_ci, image_view_ci, sampler_ci, name) {

    upload_texture_data(texture_data, texture_size);
    generate_mipmaps();
}

GpuTexture::GpuTexture(const wrapper::Device &device, const VkImageCreateInfo image_ci,
                       const VkImageViewCreateInfo image_view_ci, const VkSamplerCreateInfo sampler_ci,
                       const std::string name)
    : m_device(device), m_image_ci(image_ci), m_image_view_ci(image_view_ci), m_sampler_ci(sampler_ci),
      m_name(name), wrapper::Image(device, image_ci, image_view_ci, name) {

    m_sampler = std::make_unique<Sampler>(m_device, m_sampler_ci, m_name);

    descriptor_image_info.sampler = m_sampler->sampler();
}

GpuTexture::GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture, const VkImageCreateInfo image_ci,
                       const VkImageViewCreateInfo image_view_ci, const VkSamplerCreateInfo sampler_ci)
    : GpuTexture(device, cpu_texture.data(), cpu_texture.data_size(), image_ci, image_view_ci, sampler_ci,
                 cpu_texture.name()) {}

GpuTexture::GpuTexture(const wrapper::Device &device, const VkFormat format, const CpuTexture &cpu_texture)
    : GpuTexture(device, cpu_texture, fill_image_ci(format, cpu_texture.width(), cpu_texture.height()),
                 fill_image_view_ci(format), fill_sampler_ci(device)) {}

GpuTexture::GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture)
    : GpuTexture(device, cpu_texture, fill_image_ci(DEFAULT_TEXTURE_FORMAT, cpu_texture.width(), cpu_texture.height()),
                 fill_image_view_ci(DEFAULT_TEXTURE_FORMAT), fill_sampler_ci(device)) {}

GpuTexture::GpuTexture(const wrapper::Device &device) : GpuTexture(device, CpuTexture()) {}

void GpuTexture::upload_texture_data(const void *texture_data, const std::size_t texture_size) {
    assert(texture_data);
    assert(texture_size > 0);

    wrapper::StagingBuffer texture_staging_buffer(m_device, m_name, texture_size, texture_data, texture_size);

    wrapper::OnceCommandBuffer copy_command(m_device, m_device.graphics_queue(), m_device.graphics_queue_family_index(),
                                            [&](const wrapper::CommandBuffer &cmd_buf) {
                                                change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                                                copy_from_buffer(cmd_buf, texture_staging_buffer.buffer(),
                                                                 m_image_ci.extent.width, m_image_ci.extent.height);
                                                change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                                            });
}

void GpuTexture::generate_mipmaps() {
    // TODO: Only one command pool per thread!
    wrapper::CommandPool cmd_pool(m_device);
    wrapper::CommandBuffer cmd_buf(m_device, cmd_pool.get(), "test");

    cmd_buf.begin_command_buffer();

    change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image_ci.mipLevels);

    // Note that when generating mip levels here, we start with index 1
    for (std::uint32_t mip_level = 1; mip_level < m_image_ci.mipLevels; mip_level++) {

        VkImageBlit imageBlit{};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel = mip_level - 1;
        imageBlit.srcOffsets[1].x = static_cast<std::int32_t>(m_image_ci.extent.width >> (mip_level - 1));
        imageBlit.srcOffsets[1].y = static_cast<std::int32_t>(m_image_ci.extent.height >> (mip_level - 1));
        imageBlit.srcOffsets[1].z = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel = mip_level;
        imageBlit.dstOffsets[1].x = static_cast<std::int32_t>(m_image_ci.extent.width >> mip_level);
        imageBlit.dstOffsets[1].y = static_cast<std::int32_t>(m_image_ci.extent.height >> mip_level);
        imageBlit.dstOffsets[1].z = 1;

        change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1,
                            mip_level);

        vkCmdBlitImage(cmd_buf.get(), image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

        change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1,
                            mip_level);
    }

    change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        m_image_ci.mipLevels);

    cmd_buf.flush_command_buffer_and_wait();

    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

GpuTexture::GpuTexture(GpuTexture &&other) noexcept : m_device(other.m_device), wrapper::Image(std::move(other)) {
    m_sampler = std::exchange(other.m_sampler, nullptr);
    m_image_ci = std::move(other.m_image_ci);
    m_name = std::move(other.m_name);
    m_image_view_ci = std::move(other.m_image_view_ci);
    m_sampler_ci = std::move(other.m_sampler_ci);
}

} // namespace inexor::vulkan_renderer::texture
