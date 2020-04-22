#include "inexor/vulkan-renderer/texture_manager.hpp"

// stb single-file public domain libraries for C/C++
// https://github.com/nothings/stb
// License: Public Domain
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace inexor::vulkan_renderer {

VkResult VulkanTextureManager::init(const VkDevice &device, const VkPhysicalDevice &graphics_card,
                                    const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const VmaAllocator &vma_allocator,
                                    const uint32_t &transfer_queue_family_index, const VkQueue &data_transfer_queue) {
    assert(device);
    assert(vma_allocator);
    assert(data_transfer_queue);
    assert(debug_marker_manager);
    assert(graphics_card);

    // All the other variables will be stored in the base class from which we inherit.
    this->vma_allocator = vma_allocator;
    this->graphics_card = graphics_card;
    this->transfer_queue_family_index = transfer_queue_family_index;

    // Initialise base class.
    SingleTimeCommandBufferRecorder::init(device, debug_marker_manager, data_transfer_queue);

    spdlog::debug("Initialising Vulkan texture buffer manager.");
    spdlog::debug("Creating command pool for texture buffer manager.");

    create_texture_manager_command_pool();

    texture_manager_initialised = true;

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_manager_command_pool() {
    assert(device);

    VkCommandPoolCreateInfo command_pool_create_info = {};

    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;

    // TODO: Do we need this?
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // This might be a distinct data transfer queue.
    command_pool_create_info.queueFamilyIndex = transfer_queue_family_index;

    // Create a second command pool for all commands that are going to be executed in the data transfer queue.
    VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
    vulkan_error_check(result);

    // Give this command pool an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_pool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
                                          "Command pool for VulkanTextureManager.");

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = data_transfer_command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    spdlog::debug("Allocating command buffers for texture buffer manager.");

    // Allocate a command buffer for data transfer commands.
    result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
    vulkan_error_check(result);

    // Give this command pool an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_buffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                          "Command buffer for VulkanTextureManager.");

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_buffer(std::shared_ptr<InexorTexture> texture, InexorBuffer &buffer_object, const VkDeviceSize &buffer_size,
                                                     const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage) {
    assert(vma_allocator);
    assert(debug_marker_manager);
    assert(!texture->name.empty());

    spdlog::debug("Creating data buffer for texture '{}.'", texture->name);

    buffer_object.create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_object.create_info.size = buffer_size;
    buffer_object.create_info.usage = buffer_usage;
    buffer_object.create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    buffer_object.allocation_create_info.usage = memory_usage;
    buffer_object.allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    buffer_object.allocation_create_info.pUserData = texture->name.data();

    VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object.create_info, &buffer_object.allocation_create_info, &buffer_object.buffer,
                                      &buffer_object.allocation, &buffer_object.allocation_info);
    vulkan_error_check(result);

    // Give this texture data buffer an appropriate name.
    const std::string data_buffer_name = "Data buffer for texture '" + texture->name + "'.";

    // Give this texture buffer an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(buffer_object.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, data_buffer_name.c_str());

    return result;
}

VkResult VulkanTextureManager::create_texture_image(std::shared_ptr<InexorTexture> texture, const VkFormat &format, const VkImageTiling &tiling,
                                                    const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage,
                                                    const VkImageUsageFlags &image_usage_flags) {
    assert(texture->width > 0);
    assert(texture->height > 0);
    assert(texture->mip_levels >= 1);

    texture->image_create_info = {};

    texture->image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    texture->image_create_info.imageType = VK_IMAGE_TYPE_2D;
    texture->image_create_info.extent.width = texture->width; // TODO: Check if those are set when calling create_texture_from_file()!
    texture->image_create_info.extent.height = texture->height;
    texture->image_create_info.extent.depth = 1;
    texture->image_create_info.mipLevels = texture->mip_levels;
    texture->image_create_info.arrayLayers = 1;
    texture->image_create_info.format = format;
    texture->image_create_info.tiling = tiling;
    texture->image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    texture->image_create_info.usage = image_usage_flags;
    texture->image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    texture->image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Image creation does not automatically allocate memory for the image automatically.
    // This is done in the following code part:

    texture->allocation_create_info.usage = memory_usage;
    texture->allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    texture->allocation_create_info.pUserData = texture->name.data();

    VkResult result = vmaCreateImage(vma_allocator, &texture->image_create_info, &texture->allocation_create_info, &texture->image, &texture->allocation,
                                     &texture->allocation_info);
    vulkan_error_check(result);

    std::string image_name = "Image for texture '" + texture->name + "'.";

    // Assign an appropriate name to this image view.
    debug_marker_manager->set_object_name(device, (uint64_t)(texture->image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_name.c_str());

    return VK_SUCCESS;
}

// TODO: Overload this method for more control if necessary!
VkResult VulkanTextureManager::create_texture_image_view(std::shared_ptr<InexorTexture> texture, const VkFormat &format) {
    texture->view_create_info = {};

    texture->view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    texture->view_create_info.image = texture->image;
    texture->view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    texture->view_create_info.format = format;
    texture->view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // TODO: Refactor
    texture->view_create_info.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    texture->view_create_info.subresourceRange.baseMipLevel = 0;
    texture->view_create_info.subresourceRange.levelCount = texture->mip_levels; // TOOD: Validate that create_texture_from_file works!
    texture->view_create_info.subresourceRange.baseArrayLayer = 0;
    texture->view_create_info.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(device, &texture->view_create_info, nullptr, &texture->image_view);
    vulkan_error_check(result);

    std::string debug_marker_name = "Image view for texture '" + texture->name + "'";

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(texture->image_view), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, debug_marker_name.c_str());

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_from_memory(const std::string &internal_texture_name, void *texture_memory,
                                                          const VkDeviceSize &texture_memory_size, std::shared_ptr<InexorTexture> output_texture) {
    // This buffer is used as a transfer source for the buffer copy.
    InexorBuffer staging_buffer_for_texture;

    // Create a staging buffer for the texture data.
    VkResult result =
        create_texture_buffer(output_texture, staging_buffer_for_texture, texture_memory_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    vulkan_error_check(result);

    // Copy memory to staging buffer.
    std::memcpy(staging_buffer_for_texture.allocation_info.pMappedData, texture_memory, static_cast<std::size_t>(texture_memory_size));

    result = create_texture_image(output_texture, output_texture->format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vulkan_error_check(result);

    // TODO: Explain why we need to do this!
    transition_image_layout(output_texture->image, output_texture->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copy_buffer_to_image(staging_buffer_for_texture.buffer, output_texture->image, output_texture->width, output_texture->height);

    // TODO: Explain why we need to do this!
    transition_image_layout(output_texture->image, output_texture->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Destroy the staging buffer for the texture data.
    vmaDestroyBuffer(vma_allocator, staging_buffer_for_texture.buffer, staging_buffer_for_texture.allocation);

    // Create an image view for the texture.
    create_texture_image_view(output_texture, output_texture->format);

    // Create a texture sampler for the texture.
    create_texture_sampler(output_texture);

    // Update the texture's descriptor.
    output_texture->update_descriptor();

    // Call template base class method.
    add_entry(internal_texture_name, output_texture);

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_from_file(const std::string &internal_texture_name, const std::string &texture_file_name,
                                                        std::shared_ptr<InexorTexture> output_texture) {
    assert(texture_manager_initialised);
    assert(!internal_texture_name.empty());
    assert(!texture_file_name.empty());

    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;

    if (does_key_exist(internal_texture_name)) {
        spdlog::error("A texture with the internal name '{}' already exists!", internal_texture_name);
        output_texture = nullptr;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    spdlog::debug("Loading texture '{}' from file file '{}'.", internal_texture_name, texture_file_name);

    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    stbi_uc *pixels = stbi_load(texture_file_name.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

    if (!pixels) {
        spdlog::error("Texture file '{}' could not be loaded!", texture_file_name);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    spdlog::debug("Texture dimensions: width: {}, height: {}", texture_width, texture_height);

    // Create a staging buffer for the texture.
    VkBuffer texture_staging_buffer = VK_NULL_HANDLE;

    // Calculate the memory size of the texture.
    // We need 4 times the size since we have 4 channels: red, green, blue and alpha channel.
    VkDeviceSize texture_memory_size = 4 * texture_width * texture_height;

    std::shared_ptr<InexorTexture> new_texture = std::make_shared<InexorTexture>();

    new_texture->name = texture_file_name;
    new_texture->format = format;
    new_texture->width = texture_width;
    new_texture->height = texture_height;
    new_texture->mip_levels = 1;

    // Now that we have loaded the data from file into memory, we can call this function to continue.
    VkResult result = create_texture_from_memory(internal_texture_name, pixels, texture_memory_size, new_texture);
    vulkan_error_check(result);

    // We now can discard the image data since we copied it already.
    stbi_image_free(pixels);

    // Store the new texture to output.
    output_texture = new_texture;

    // We do call add_entry() in here because create_texture_from_memory will do this for us!
    // The method would fail anyways because the internal texture name would be duplicated.

    // TODO: Generate mip-maps!

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_from_glTF2_image(const std::string &internal_texture_name, tinygltf::Image &gltf_image,
                                                               std::shared_ptr<InexorTexture> output_texture) {
    assert(texture_manager_initialised);
    assert(!internal_texture_name.empty());

    if (does_key_exist(internal_texture_name)) {
        spdlog::error("A texture with the internal name '{}' already exists!", internal_texture_name);
        output_texture = nullptr;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::shared_ptr<InexorTexture> new_texture = std::make_shared<InexorTexture>();

    unsigned char *texture_buffer = nullptr;

    std::vector<unsigned char> texture_buffer_2;

    VkDeviceSize texture_memory_size = 0;

    if (3 == gltf_image.component) {
        // Most devices don't support RGB only on Vulkan so convert if necessary
        // TODO: Check actual format support and transform only if required
        texture_memory_size = gltf_image.width * gltf_image.height * 4;

        texture_buffer_2.resize(texture_memory_size);

        unsigned char *rgba = texture_buffer_2.data();

        unsigned char *rgb = &gltf_image.image[0];

        for (int32_t i = 0; i < gltf_image.width * gltf_image.height; ++i) {
            for (int32_t j = 0; j < 3; ++j) {
                rgba[j] = rgb[j];
            }

            rgba += 4;
            rgb += 3;
        }
    } else {
        texture_buffer = &gltf_image.image[0];

        texture_memory_size = gltf_image.image.size();
    }

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    new_texture->name = internal_texture_name;
    new_texture->format = format;
    new_texture->width = gltf_image.width;
    new_texture->height = gltf_image.height;
    new_texture->mip_levels = 1; // static_cast<uint32_t>(floor(log2(std::max(new_texture->width, new_texture->height))) + 1.0);

    // Now that we have loaded the texture's memory, we can simply call this method to continue!
    VkResult result = create_texture_from_memory(internal_texture_name, texture_buffer, texture_memory_size, new_texture);
    vulkan_error_check(result);

    // We do call add_entry() in here because create_texture_from_memory will do this for us!
    // The method would fail anyways because the internal texture name would be duplicated.

    // TODO: Generate mip-maps!

    // Store the output texture.
    output_texture = new_texture;

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::create_texture_sampler(std::shared_ptr<InexorTexture> texture) {
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

    // TODO: Verify if we should use something else than this.
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

    VkResult result = vkCreateSampler(device, &sampler_create_info, nullptr, &texture->sampler);
    vulkan_error_check(result);

    const std::string texture_sampler_name = "Texture sampler for texture '" + texture->name + "'.";

    // Give this texture sampler an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(texture->sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, texture_sampler_name.c_str());

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::transition_image_layout(VkImage &image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
    start_recording_of_single_time_command_buffer();

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

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    // TODO: Refactor!
    if (VK_IMAGE_LAYOUT_UNDEFINED == old_layout && VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == new_layout) {
        //
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == old_layout && VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == new_layout) {
        //
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        spdlog::error("Error: unsupported layout transition!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    spdlog::debug("Recording pipeline barrier for image layer transition");

    vkCmdPipelineBarrier(data_transfer_command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    end_recording_of_single_time_command_buffer();

    return VK_SUCCESS;
}

VkResult VulkanTextureManager::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    start_recording_of_single_time_command_buffer();

    VkBufferImageCopy buffer_image_region = {};

    buffer_image_region.bufferOffset = 0;
    buffer_image_region.bufferRowLength = 0;
    buffer_image_region.bufferImageHeight = 0;
    buffer_image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_image_region.imageSubresource.mipLevel = 0;
    buffer_image_region.imageSubresource.baseArrayLayer = 0;
    buffer_image_region.imageSubresource.layerCount = 1;
    buffer_image_region.imageOffset = {0, 0, 0};
    buffer_image_region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(data_transfer_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_region);

    end_recording_of_single_time_command_buffer();

    return VK_SUCCESS;
}

std::optional<std::shared_ptr<InexorTexture>> VulkanTextureManager::get_texture(const std::string &internal_texture_name) {
    if (!does_key_exist(internal_texture_name)) {
        spdlog::error("Could not find texture '{}'!", internal_texture_name);
        return std::nullopt;
    }

    auto texture = get_entry(internal_texture_name);

    if (texture.has_value()) {
        // Return the texture's image view.
        return texture.value();
    } else {
        spdlog::error("Manager class returned std::nullopt for get_entry('{}')!", internal_texture_name);
    }

    return std::nullopt;
}

std::optional<VkImageView> VulkanTextureManager::get_texture_view(const std::string &internal_texture_name) {
    if (!does_key_exist(internal_texture_name)) {
        spdlog::error("Could not find image view for texture '{}' because this texture does not exist!", internal_texture_name);
        return std::nullopt;
    }

    auto texture = get_entry(internal_texture_name);

    if (texture.has_value()) {
        // Return the texture's image view.
        return texture.value()->image_view;
    } else {
        spdlog::error("Manager class returned std::nullopt for get_entry('{}')!", internal_texture_name);
    }

    return std::nullopt;
}

std::optional<VkSampler> VulkanTextureManager::get_texture_sampler(const std::string &internal_texture_name) {
    if (!does_key_exist(internal_texture_name)) {
        spdlog::error("Could not find sampler for texture '{}' because this texture does not exist!", internal_texture_name);
        return std::nullopt;
    }

    auto texture = get_entry(internal_texture_name);

    if (texture.has_value()) {
        // Return the texture's image sampler.
        return texture.value()->sampler;
    } else {
        spdlog::error("Manager class returned std::nullopt for get_entry('{}')!", internal_texture_name);
    }

    return std::nullopt;
}

void VulkanTextureManager::shutdown_textures() {
    auto textures = get_all_values();

    for (const auto &texture : textures) {
        texture->destroy_texture(device, vma_allocator);
    }

    destroy_command_pool();
}

} // namespace inexor::vulkan_renderer
