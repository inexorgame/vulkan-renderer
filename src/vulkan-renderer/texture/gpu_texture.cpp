#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"

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
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.format = format;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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
    : GpuTexture(device, cpu_texture, fill_image_ci(DEFAULT_FORMAT, cpu_texture.width(), cpu_texture.height()),
                 fill_image_view_ci(DEFAULT_FORMAT), fill_sampler_ci(device)) {}

GpuTexture::GpuTexture(const wrapper::Device &device) : GpuTexture(device, CpuTexture()) {}

void GpuTexture::upload_texture_data(const void *texture_data, const std::size_t texture_size) {
    assert(texture_data);
    assert(texture_size > 0);

    wrapper::StagingBuffer texture_staging_buffer(m_device, m_name, texture_size, texture_data, texture_size);

    wrapper::OnceCommandBuffer copy_command(m_device, m_device.graphics_queue(), m_device.graphics_queue_family_index(),
                                            [&](const wrapper::CommandBuffer &cmd_buf) {
                                                change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                                                copy_from_buffer(cmd_buf, texture_staging_buffer.buffer(),
                                                                 m_image_ci.extent.width, m_image_ci.extent.height);
                                                change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                                            });
}

GpuTexture::GpuTexture(GpuTexture &&other) noexcept : m_device(other.m_device), wrapper::Image(std::move(other)) {}

} // namespace inexor::vulkan_renderer::texture
