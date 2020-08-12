#include "inexor/vulkan-renderer/wrapper/texture.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <vma/vma_usage.h>

namespace inexor::vulkan_renderer::wrapper {

Texture::Texture(wrapper::Device &device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
                 void *texture_data, const std::size_t texture_size, const std::string &name,
                 const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index)
    : m_name(name), m_file_name(m_file_name), m_device(device), m_graphics_card(graphics_card),
      m_data_transfer_queue(data_transfer_queue), m_vma_allocator(vma_allocator),
      m_copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index) {

    create_texture(texture_data, texture_size);
}

Texture::Texture(wrapper::Device &device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
                 const std::string &file_name, const std::string &name, const VkQueue data_transfer_queue,
                 const std::uint32_t data_transfer_queue_family_index)
    : m_name(name), m_file_name(file_name), m_device(device), m_graphics_card(graphics_card),
      m_data_transfer_queue(data_transfer_queue), m_data_transfer_queue_family_index(data_transfer_queue_family_index),
      m_vma_allocator(vma_allocator),
      m_copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index) {
    assert(device.device());
    assert(vma_allocator);
    assert(!file_name.empty());
    assert(!name.empty());
    assert(data_transfer_queue);

    spdlog::debug("Loading texture file {}.", file_name);

    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    stbi_uc *texture_data =
        stbi_load(file_name.c_str(), &m_texture_width, &m_texture_height, &m_texture_channels, STBI_rgb_alpha);

    if (texture_data == nullptr) {
        throw std::runtime_error("Error: Could not load texture file " + file_name + " using stbi_load!");
    }

    spdlog::debug("Texture dimensions: width: {}, height: {}, channels: {}.", m_texture_width, m_texture_height,
                  m_texture_channels);

    // Calculate the memory size of the texture.
    // We need 4 times the size since we have 4 channels: red, green, blue and alpha channel.
    VkDeviceSize texture_memory_size = 4 * m_texture_width * m_texture_height;

    create_texture(texture_data, texture_memory_size);

    // We can discard the texture data since we copied it already to GPU memory.
    stbi_image_free(texture_data);
}

Texture::Texture(Texture &&other) noexcept
    : m_texture_image(std::exchange(other.m_texture_image, nullptr)), m_name(std::move(other.m_name)),
      m_file_name(std::move(other.m_file_name)), m_texture_width(other.m_texture_width),
      m_texture_height(other.m_texture_height), m_texture_channels(other.m_texture_channels),
      m_mip_levels(other.m_mip_levels), m_device(other.m_device), m_graphics_card(other.m_graphics_card),
      m_data_transfer_queue(other.m_data_transfer_queue), m_vma_allocator(other.m_vma_allocator),
      m_sampler(std::exchange(other.m_sampler, nullptr)), m_texture_image_format(other.m_texture_image_format),
      m_copy_command_buffer(std::move(other.m_copy_command_buffer)) {}

Texture::~Texture() {
    spdlog::trace("Destroying texture {}.", m_name);
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

void Texture::create_texture(void *texture_data, const std::size_t texture_size) {

    // For now, we will not generate mip-maps automatically.
    // TODO: Generate mip-maps automatically!
    m_mip_levels = 1;

    StagingBuffer texture_staging_buffer(m_device, m_vma_allocator, m_data_transfer_queue,
                                         m_data_transfer_queue_family_index, m_name, texture_size, texture_data,
                                         texture_size);

    VkExtent2D extent;
    extent.width = m_texture_width;
    extent.height = m_texture_height;

    m_texture_image =
        std::make_unique<wrapper::Image>(m_device, m_graphics_card, m_vma_allocator, m_texture_image_format,
                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, m_name, extent);

    m_copy_command_buffer.create_command_buffer();

    m_copy_command_buffer.start_recording();

    spdlog::debug("Transitioning image layout of texture {} to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.", m_name);

    transition_image_layout(m_texture_image->get(), m_texture_image_format, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy buffer_image_region{};

    buffer_image_region.bufferOffset = 0;
    buffer_image_region.bufferRowLength = 0;
    buffer_image_region.bufferImageHeight = 0;
    buffer_image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_image_region.imageSubresource.mipLevel = 0;
    buffer_image_region.imageSubresource.baseArrayLayer = 0;
    buffer_image_region.imageSubresource.layerCount = 1;
    buffer_image_region.imageOffset = {0, 0, 0};
    buffer_image_region.imageExtent = {static_cast<uint32_t>(m_texture_width), static_cast<uint32_t>(m_texture_height),
                                       1};

    vkCmdCopyBufferToImage(m_copy_command_buffer.command_buffer(), texture_staging_buffer.buffer(),
                           m_texture_image->get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_region);

    spdlog::debug("Transitioning image layout of texture {} to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.", m_name);

    m_copy_command_buffer.end_recording_and_submit_command();

    transition_image_layout(m_texture_image->get(), m_texture_image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    create_texture_sampler();
}

void Texture::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout,
                                      VkImageLayout new_layout) {

    auto barrier = make_info<VkImageMemoryBarrier>();
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage = 0;
    VkPipelineStageFlags destination_stage = 0;

    if (VK_IMAGE_LAYOUT_UNDEFINED == old_layout && VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == new_layout) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == old_layout &&
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == new_layout) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::runtime_error("Error: unsupported layout transition!");
    }

    spdlog::debug("Recording pipeline barrier for image layer transition");

    OnceCommandBuffer image_transition_change(m_device, m_data_transfer_queue, m_data_transfer_queue_family_index);

    image_transition_change.create_command_buffer();
    image_transition_change.start_recording();

    vkCmdPipelineBarrier(image_transition_change.command_buffer(), source_stage, destination_stage, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    image_transition_change.end_recording_and_submit_command();
}

void Texture::create_texture_sampler() {
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

    // The unnormalizedCoordinates field specifies which coordinate system you
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
    vkGetPhysicalDeviceFeatures(m_graphics_card, &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(m_graphics_card, &graphics_card_properties);

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

    if (vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSampler failed for texture " + m_name + " !");
    }

#ifndef NDEBUG
    // Assign an internal name using Vulkan debug markers.
    m_device.set_object_name(m_sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_name);
#endif

    spdlog::debug("Image sampler {} created successfully.", m_name);
}

} // namespace inexor::vulkan_renderer::wrapper
