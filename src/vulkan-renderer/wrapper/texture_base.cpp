#include "inexor/vulkan-renderer/wrapper/texture_base.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

namespace inexor::vulkan_renderer::wrapper {

VkSamplerCreateInfo default_sampler_settings(const VkPhysicalDevice gpu) {
    auto sampler_ci = make_info<VkSamplerCreateInfo>();

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

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(gpu, &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(gpu, &graphics_card_properties);

    if (device_features.samplerAnisotropy) {
        sampler_ci.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_ci.anisotropyEnable = VK_TRUE;
    } else {
        sampler_ci.maxAnisotropy = 1.0;
        sampler_ci.anisotropyEnable = VK_FALSE;
    }

    return sampler_ci;
}

TextureBase::TextureBase(const wrapper::Device &device) : m_device(device) {}

void TextureBase::create_default_texture_sampler() {
    create_texture_sampler(default_sampler_settings(m_device.physical_device()));
}

void TextureBase::create_texture_sampler(const VkSamplerCreateInfo sampler_ci) {
    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed for texture " + m_attributes.name + " !", result);
    }

    m_device.set_debug_marker_name(m_sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_attributes.name);
}

void TextureBase::create_image(const void *texture_data, const std::size_t texture_size) {
    assert(texture_data);
    assert(texture_size > 0);

    StagingBuffer texture_staging_buffer(m_device, m_attributes.name, texture_size, texture_data, texture_size);

    m_image = std::make_unique<Image>(m_device, m_attributes.format, m_attributes.width, m_attributes.height,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT, "unnamed texture");

    wrapper::OnceCommandBuffer copy_command(m_device, m_device.graphics_queue(),
                                            m_device.graphics_queue_family_index());

    copy_command.create_command_buffer();
    copy_command.start_recording();
    {
        m_image->transition_image_layout(copy_command.command_buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        m_image->copy_from_buffer(copy_command.command_buffer(), texture_staging_buffer.buffer(), m_attributes.width,
                                  m_attributes.height);

        m_image->transition_image_layout(copy_command.command_buffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    copy_command.end_recording_and_submit_command();
}

void TextureBase::update_descriptor() {
    m_descriptor.sampler = m_sampler;
    m_descriptor.imageView = m_image->image_view();
    m_descriptor.imageLayout = m_image->image_layout();
}

} // namespace inexor::vulkan_renderer::wrapper
