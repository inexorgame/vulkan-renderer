#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GpuTexture::GpuTexture(const Device &device, const CpuTexture &cpu_texture)
    : m_device(device), m_texture_width(cpu_texture.width()), m_texture_height(cpu_texture.height()),
      m_texture_channels(cpu_texture.channels()), m_mip_levels(cpu_texture.mip_levels()), m_name(cpu_texture.name()),
      m_copy_command_buffer(device, device.graphics_queue(), device.graphics_queue_family_index()) {
    create_texture(cpu_texture.data(), cpu_texture.data_size());
    // TODO: Do we even need to create a texture sampler in this case?
    create_texture_sampler();
}

GpuTexture::GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const CpuTexture &cpu_texture)
    : m_device(device), m_texture_width(cpu_texture.width()), m_texture_height(cpu_texture.height()),
      m_texture_channels(cpu_texture.channels()), m_mip_levels(cpu_texture.mip_levels()), m_name(cpu_texture.name()),
      m_copy_command_buffer(device, device.graphics_queue(), device.graphics_queue_family_index()) {
    create_texture(cpu_texture.data(), cpu_texture.data_size());
    create_texture_sampler(sampler);
}

GpuTexture::GpuTexture(const Device &device, const void *data, std::size_t data_size, std::uint32_t texture_width,
                       std::uint32_t texture_height, std::uint32_t texture_channels, std::uint32_t mip_levels,
                       std::string name)
    : m_device(device), m_texture_width(texture_width), m_texture_height(texture_height),
      m_texture_channels(texture_channels), m_mip_levels(mip_levels), m_name(std::move(name)),
      m_copy_command_buffer(device, device.graphics_queue(), device.graphics_queue_family_index()) {
    create_texture(data, data_size);
    create_texture_sampler();
}

GpuTexture::GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const void *data,
                       std::size_t data_size, std::uint32_t texture_width, std::uint32_t texture_height,
                       std::uint32_t texture_channels, std::uint32_t mip_levels, std::string name)

    : m_device(device), m_texture_width(texture_width), m_texture_height(texture_height),
      m_texture_channels(texture_channels), m_mip_levels(mip_levels), m_name(std::move(name)),
      m_copy_command_buffer(device, device.graphics_queue(), device.graphics_queue_family_index()) {

    create_texture(data, data_size);
    // Create the texture sampler from the settings specified in the glTF2 file.
    create_texture_sampler(sampler);
}

GpuTexture::GpuTexture(GpuTexture &&other) noexcept
    : m_device(other.m_device), m_texture_image_format(other.m_texture_image_format),
      m_copy_command_buffer(std::move(other.m_copy_command_buffer)) {
    m_texture_image = std::exchange(other.m_texture_image, nullptr);
    m_name = std::move(other.m_name);
    m_texture_width = other.m_texture_width;
    m_texture_height = other.m_texture_height;
    m_texture_channels = other.m_texture_channels;
    m_mip_levels = other.m_mip_levels;
    m_sampler = std::exchange(other.m_sampler, nullptr);
}

GpuTexture::~GpuTexture() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

void GpuTexture::create_texture(const void *texture_data, const std::size_t texture_size) {

    StagingBuffer texture_staging_buffer(m_device, m_name, texture_size, texture_data, texture_size);

    m_texture_image = std::make_unique<Image>(m_device, m_texture_image_format, m_texture_width, m_texture_height,
                                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, "texture");

    m_copy_command_buffer.create_command_buffer();
    m_copy_command_buffer.start_recording();

    m_texture_image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_texture_image->copy_from_buffer(m_copy_command_buffer.command_buffer(), texture_staging_buffer.buffer(),
                                      m_texture_width, m_texture_height);

    m_copy_command_buffer.end_recording_and_submit_command();

    m_texture_image->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void GpuTexture::create_texture_sampler(const gltf::TextureSampler &sampler) {
    auto sampler_ci = make_info<VkSamplerCreateInfo>();
    sampler_ci.magFilter = sampler.mag_filter();
    sampler_ci.minFilter = sampler.min_filter();
    sampler_ci.addressModeU = sampler.address_mode_u();
    sampler_ci.addressModeV = sampler.address_mode_v();
    sampler_ci.addressModeW = sampler.address_mode_w();

    // These two fields specify if anisotropic filtering should be used.
    // There is no reason not to use this unless performance is a concern.
    // The maxAnisotropy field limits the amount of texel samples that can
    // be used to calculate the final color. A lower value results in better
    // performance, but lower quality results. There is no graphics hardware
    // available today that will use more than 16 samples, because the difference
    // is negligible beyond that point.
    sampler_ci.anisotropyEnable = VK_TRUE;
    sampler_ci.maxAnisotropy = 16;

    // The borderColor field specifies which color is returned when sampling beyond
    // the image with clamp to border addressing mode. It is possible to return black,
    // white or transparent in either float or int formats. You cannot specify an arbitrary color.
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // The unnormalized coordinates field specifies which coordinate system you
    // want to use to address texels in an image. If this field is VK_TRUE, then you
    // can simply use coordinates within the [0, texWidth) and [0, texHeight)
    // range. If it is VK_FALSE, then the texels are addressed using the [0, 1) range
    // on all axes. Real-world applications almost always use normalized coordinates,
    // because then it's possible to use textures of varying resolutions with the exact
    // same coordinates.
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.compareEnable = VK_FALSE;
    sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(m_device.physical_device(), &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(m_device.physical_device(), &graphics_card_properties);

    if (VK_TRUE == device_features.samplerAnisotropy) {
        // Anisotropic filtering is available.
        sampler_ci.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_ci.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler_ci.maxAnisotropy = 1.0;
        sampler_ci.anisotropyEnable = VK_FALSE;
    }

    spdlog::debug("Creating image sampler for texture {}.", m_name);

    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed for texture " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_name);

    spdlog::debug("Image sampler {} created successfully.", m_name);
}

void GpuTexture::create_texture_sampler() {
    auto sampler_ci = make_info<VkSamplerCreateInfo>();
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // These two fields specify if anisotropic filtering should be used.
    // There is no reason not to use this unless performance is a concern.
    // The maxAnisotropy field limits the amount of texel samples that can
    // be used to calculate the final color. A lower value results in better
    // performance, but lower quality results. There is no graphics hardware
    // available today that will use more than 16 samples, because the difference
    // is negligible beyond that point.
    sampler_ci.anisotropyEnable = VK_TRUE;
    sampler_ci.maxAnisotropy = 16;

    // The borderColor field specifies which color is returned when sampling beyond
    // the image with clamp to border addressing mode. It is possible to return black,
    // white or transparent in either float or int formats. You cannot specify an arbitrary color.
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // The unnormalized coordinates field specifies which coordinate system you
    // want to use to address texels in an image. If this field is VK_TRUE, then you
    // can simply use coordinates within the [0, texWidth) and [0, texHeight)
    // range. If it is VK_FALSE, then the texels are addressed using the [0, 1) range
    // on all axes. Real-world applications almost always use normalized coordinates,
    // because then it's possible to use textures of varying resolutions with the exact
    // same coordinates.
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.compareEnable = VK_FALSE;
    sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(m_device.physical_device(), &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(m_device.physical_device(), &graphics_card_properties);

    if (VK_TRUE == device_features.samplerAnisotropy) {
        // Anisotropic filtering is available.
        sampler_ci.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_ci.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler_ci.maxAnisotropy = 1.0;
        sampler_ci.anisotropyEnable = VK_FALSE;
    }

    spdlog::debug("Creating image sampler for texture {}.", m_name);

    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed for texture " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_name);

    spdlog::debug("Image sampler {} created successfully.", m_name);
}

} // namespace inexor::vulkan_renderer::wrapper
