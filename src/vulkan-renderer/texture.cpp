#include "inexor/vulkan-renderer/texture.hpp"

// stb single-file public domain libraries for C/C++
// https://github.com/nothings/stb
// License: Public Domain
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace inexor::vulkan_renderer {

Texture::Texture(Texture &&other) noexcept
    : name(std::move(other.name)), file_name(std::move(other.file_name)), texture_width(other.texture_width), texture_height(other.texture_height),
      texture_channels(other.texture_channels), mip_levels(other.mip_levels), device(std::move(other.device)), graphics_card(std::move(other.graphics_card)),
      data_transfer_queue(std::move(other.data_transfer_queue)), vma_allocator(std::move(other.vma_allocator)), allocation(std::move(other.allocation)),
      allocation_info(std::move(other.allocation_info)), image(std::move(other.image)), image_view(std::move(other.image_view)),
      sampler(std::move(other.sampler)), texture_image_format(other.texture_image_format), copy_command_buffer(std::move(other.copy_command_buffer)) {}

// TODO: Remove unnecessary parameters!
Texture::Texture(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator, void *texture_data,
                 const std::size_t texture_size, std::string &name, const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index)
    : name(name), file_name(file_name), device(device), graphics_card(graphics_card), data_transfer_queue(data_transfer_queue), vma_allocator(vma_allocator),
      copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index) {
    StagingBuffer texture_staging_buffer(device, vma_allocator, copy_command_buffer.get_command_buffer(), data_transfer_queue, data_transfer_queue_family_index,
                                         name, texture_size, texture_data, texture_size);

    VkImageCreateInfo image_create_info = {};

    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = texture_width;
    image_create_info.extent.height = texture_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = texture_image_format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // TODO: Check if other memory flags are necessary, especially for the case of VMA_RECORDING_ENABLED not being defined!

#if VMA_RECORDING_ENABLED
    allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocation_create_info.pUserData = name.data();
#endif

    if (vmaCreateImage(vma_allocator, &image_create_info, &allocation_create_info, &image, &allocation, &allocation_info) != VK_SUCCESS) {
        throw std::runtime_error("Error: vmaCreateImage failed for texture " + name + " !");
    }

    transition_image_layout(image, texture_image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copy_command_buffer.start_recording();

    VkBufferImageCopy buffer_image_region = {};

    buffer_image_region.bufferOffset = 0;
    buffer_image_region.bufferRowLength = 0;
    buffer_image_region.bufferImageHeight = 0;
    buffer_image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_image_region.imageSubresource.mipLevel = 0;
    buffer_image_region.imageSubresource.baseArrayLayer = 0;
    buffer_image_region.imageSubresource.layerCount = 1;
    buffer_image_region.imageOffset = {0, 0, 0};
    buffer_image_region.imageExtent = {static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height), 1};

    vkCmdCopyBufferToImage(copy_command_buffer.get_command_buffer(), texture_staging_buffer.get_buffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &buffer_image_region);

    copy_command_buffer.end_recording_and_submit_command();

    transition_image_layout(image, texture_image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    create_texture_image_view();

    create_texture_sampler();
}

Texture::Texture(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator, const std::string &file_name, std::string &name,
                 const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index)
    : name(name), file_name(file_name), device(device), graphics_card(graphics_card), data_transfer_queue(data_transfer_queue),
      data_transfer_queue_family_index(data_transfer_queue_family_index), vma_allocator(vma_allocator),
      copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index) {
    assert(device);
    assert(vma_allocator);
    assert(!file_name.empty());
    assert(!name.empty());
    assert(data_transfer_queue);

    spdlog::debug("Loading texture file {}.", file_name);

    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    stbi_uc *texture_data = stbi_load(file_name.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

    if (texture_data == nullptr) {
        throw std::runtime_error("Error: Could not load texture file " + file_name + " using stbi_load!");
    }

    spdlog::debug("Texture dimensions: width: {}, height: {}, channels: {}.", texture_width, texture_height, texture_channels);

    // Calculate the memory size of the texture.
    // We need 4 times the size since we have 4 channels: red, green, blue and alpha channel.
    VkDeviceSize texture_memory_size = 4 * texture_width * texture_height;

    // For now, we will not generate mip-maps automatically.
    // TODO: Generate mip-maps automatically!
    mip_levels = 1;

    // We can discard the texture data since we copied it already to GPU memory.
    stbi_image_free(texture_data);
}

void Texture::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {

    VkImageMemoryBarrier barrier = {};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == old_layout && VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == new_layout) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::runtime_error("Error: unsupported layout transition!");
    }

    spdlog::debug("Recording pipeline barrier for image layer transition");

    vkCmdPipelineBarrier(copy_command_buffer.get_command_buffer(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Texture::create_texture_image_view() {
    VkImageViewCreateInfo image_view_create_info = {};

    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = texture_image_format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = mip_levels;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    spdlog::debug("Creating image view for texture {}.", name);

    if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateImageView failed for texture " + name + " !");
    }

    // TODO: Vulkan debug markers!

    spdlog::debug("Image view created successfully.");
}

void Texture::create_texture_sampler() {
    VkSamplerCreateInfo sampler_create_info = {};

    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // These two fields specify if anisotropic filtering should be used.
    // There is no reason not to use this unless performance is a concern.
    // The maxAnisotropy field limits the amount of texel samples that can
    // be used to calculate the final color. A lower value results in better
    // performance, but lower quality results. There is no graphics hardware
    // available today that will use more than 16 samples, because the difference
    // is negligible beyond that point.
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 16;

    // The borderColor field specifies which color is returned when sampling beyond
    // the image with clamp to border addressing mode. It is possible to return black,
    // white or transparent in either float or int formats. You cannot specify an arbitrary color.
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // The unnormalizedCoordinates field specifies which coordinate system you
    // want to use to address texels in an image. If this field is VK_TRUE, then you
    // can simply use coordinates within the [0, texWidth) and [0, texHeight)
    // range. If it is VK_FALSE, then the texels are addressed using the [0, 1) range
    // on all axes. Real-world applications almost always use normalized coordinates,
    // because then it's possible to use textures of varying resolutions with the exact
    // same coordinates.
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(graphics_card, &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

    if (VK_TRUE == device_features.samplerAnisotropy) {
        // Anisotropic filtering is available.
        sampler_create_info.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_create_info.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler_create_info.maxAnisotropy = 1.0;
        sampler_create_info.anisotropyEnable = VK_FALSE;
    }

    spdlog::debug("Creating image sampler for texture {}.", name);

    if (vkCreateSampler(device, &sampler_create_info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSampler failed for texture " + name + " !");
    }

    // TODO: Vulkan debug markers!

    spdlog::debug("Image sampler created successfully.");
}

Texture::~Texture() {
    vkDestroySampler(device, sampler, nullptr);
    vmaDestroyImage(vma_allocator, image, allocation);
    vkDestroyImageView(device, image_view, nullptr);
}

} // namespace inexor::vulkan_renderer
