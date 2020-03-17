#include "VulkanTextureManager.hpp"
#include "../error-handling/VulkanErrorHandling.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanTextureManager::VulkanTextureManager()
	{
	}

	
	VulkanTextureManager::~VulkanTextureManager()
	{
	}

	VkResult VulkanTextureManager::initialise(const VkDevice& device, const VkPhysicalDevice& graphics_card, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,  const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue)
	{
		assert(device);
		assert(vma_allocator);
		assert(data_transfer_queue);
		assert(debug_marker_manager);
		assert(graphics_card);

		this->device              = device;
		this->vma_allocator       = vma_allocator;
		this->data_transfer_queue = data_transfer_queue;
		this->dbg_marker_manager  = debug_marker_manager;
		this->graphics_card       = graphics_card;

		spdlog::debug("Initialising Vulkan texture buffer manager.");
		spdlog::debug("Creating command pool for texture buffer manager.");

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext = nullptr;
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // TODO: Do we need this?

		// This might be a distinct data transfer queue.
		command_pool_create_info.queueFamilyIndex = transfer_queue_family_index;

		// Create a second command pool for all commands that are going to be executed in the data transfer queue.
		VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
		vulkan_error_check(result);
		
		// Give this command pool an appropriate name.
		dbg_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_pool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, "Command pool for VulkanTextureManager.");
		
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;

		spdlog::debug("Allocating command buffers for texture buffer manager.");

		// Allocate a command buffer for data transfer commands.
		result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
		vulkan_error_check(result);

		// Give this command pool an appropriate name.
		dbg_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_buffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Command buffer for VulkanTextureManager.");

		return result;
	}


	VkResult VulkanTextureManager::create_texture_buffer(std::shared_ptr<InexorTexture> texture, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage)
	{
		assert(vma_allocator);
		assert(dbg_marker_manager);
		assert(texture->texture_name.length()>0);
		
		spdlog::debug("Creating data buffer for texture '" + texture->texture_name + "'.");

		buffer_object.create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_object.create_info.size        = buffer_size;
		buffer_object.create_info.usage       = buffer_usage;
		buffer_object.create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		buffer_object.allocation_create_info.usage     = memory_usage;
		buffer_object.allocation_create_info.flags     = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		buffer_object.allocation_create_info.pUserData = texture->texture_name.data();

		VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object.create_info, &buffer_object.allocation_create_info, &buffer_object.buffer, &buffer_object.allocation, &buffer_object.allocation_info);
		vulkan_error_check(result);
		
		// Give this texture data buffer an appropriate name.
		const std::string data_buffer_name = "Data buffer for texture '" + texture->texture_name + "'.";

		// Give this texture buffer an appropriate name.
		dbg_marker_manager->set_object_name(device, (uint64_t)(buffer_object.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, data_buffer_name.c_str());

		return result;
	}
	

	VkResult VulkanTextureManager::create_texture_image(std::shared_ptr<InexorTexture> texture, const uint32_t& texture_width, const uint32_t& texture_height, const VkFormat& format, const VkImageTiling& tiling, const VmaMemoryUsage& memory_usage, const VkBufferUsageFlags& buffer_usage, const VkImageUsageFlags& image_usage_flags)
	{
		texture->image_create_info = {};

		texture->image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		texture->image_create_info.imageType     = VK_IMAGE_TYPE_2D;
		texture->image_create_info.extent.width  = texture_width;
		texture->image_create_info.extent.height = texture_height;
		texture->image_create_info.extent.depth  = 1;
		texture->image_create_info.mipLevels     = 1;
		texture->image_create_info.arrayLayers   = 1;
		texture->image_create_info.format        = format;
		texture->image_create_info.tiling        = tiling;
		texture->image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		texture->image_create_info.usage         = image_usage_flags;
		texture->image_create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
		texture->image_create_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
				
		// Image creation does not allocate memory for the image automatically.
		// This is done in the following code part:

		texture->allocation_create_info.usage     = memory_usage;
		texture->allocation_create_info.flags     = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		texture->allocation_create_info.pUserData = texture->texture_name.data();
		
		VkResult result = vmaCreateImage(vma_allocator, &texture->image_create_info, &texture->allocation_create_info, &texture->image, &texture->allocation, &texture->allocation_info);
		vulkan_error_check(result);

		// Assign an appropriate name to this image view.
		std::string image_name = "Image for texture '" + texture->texture_name + "'";

		dbg_marker_manager->set_object_name(device, (uint64_t)(texture->image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_name.c_str());

		return VK_SUCCESS;
	}
	

	VkResult VulkanTextureManager::begin_single_time_commands()
	{
		spdlog::debug("Started recording command buffer for single command.");

		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.pNext              = nullptr;
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		// We have to allocate the command buffer every time this function is
		// called because we call vkFreeCommandBuffers in end_single_time_commands.
		VkResult result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
		vulkan_error_check(result);

		// TODO: Assign memory marker to data_transfer_command_buffer!

		VkCommandBufferBeginInfo command_buffer_begin_info = {};

		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext = nullptr;

		// We’re only going to use the command buffer once and wait with returning from the function until
		// the copy operation has finished executing. It’s good practice to tell the driver about our intent
		// using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Begin recording of the command buffer.
		result = vkBeginCommandBuffer(data_transfer_command_buffer, &command_buffer_begin_info);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanTextureManager::end_single_time_commands()
	{
		spdlog::debug("Ended recording command buffer for single time commands.");

		VkResult result = vkEndCommandBuffer(data_transfer_command_buffer);
		vulkan_error_check(result);

		VkSubmitInfo submit_info = {};

		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &data_transfer_command_buffer;

		result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		vulkan_error_check(result);

		// TODO: Fence!
		result = vkQueueWaitIdle(data_transfer_queue);
		vulkan_error_check(result);
		
		spdlog::debug("Destroying command buffer again.");

		// Because we destroy the command buffer after submission, we have to allocate it every time begin_single_commands is called.
		vkFreeCommandBuffers(device, data_transfer_command_pool, 1, &data_transfer_command_buffer);

		return VK_SUCCESS;
	}
	
	
	VkResult VulkanTextureManager::create_texture_image_view(std::shared_ptr<InexorTexture> texture, const VkFormat& format)
	{
		texture->view_create_info = {};

		texture->view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		texture->view_create_info.image                           = texture->image;
		texture->view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		texture->view_create_info.format                          = format;
		texture->view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		texture->view_create_info.subresourceRange.baseMipLevel   = 0;
		texture->view_create_info.subresourceRange.levelCount     = 1;
		texture->view_create_info.subresourceRange.baseArrayLayer = 0;
		texture->view_create_info.subresourceRange.layerCount     = 1;
		
		VkResult result = vkCreateImageView(device, &texture->view_create_info, nullptr, &texture->view);
		vulkan_error_check(result);

		return VK_SUCCESS;
	}


	VkResult VulkanTextureManager::create_texture_from_file(const std::string& texture_name, const std::string& file_name, std::shared_ptr<InexorTexture> texture)
	{
		int texture_width = 0;
		int texture_height = 0;
		int texture_channels = 0;
		
		// TODO: Check if texture with this name does already exist.

		const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		spdlog::debug("Loading texture {}.", file_name);
		
		// Load the texture file using stb_image library.
		// Force stb_image to load an alpha channel as well.
		stbi_uc* pixels = stbi_load(file_name.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

		if(!pixels)
		{
			spdlog::error("Texture {} could not be loaded!", file_name);
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		spdlog::debug("Texture width: {}, height: {}", texture_width, texture_height);

		// Note: Inexor vulkan-renderer does not intend to support linear tiled textures because it is not advisable to do so!

		// Create a staging buffer for the texture.
		VkBuffer texture_staging_buffer = VK_NULL_HANDLE;

		// Calculate the memory size of the texture.
		// We need 4 times the size since we have 4 channels: red, green, blue and alpha channel.
		VkDeviceSize texture_memory_size = 4 * texture_width * texture_height;
		
		// Store the name of the texture.
		texture->texture_name = texture_name;

		// Create a staging vertex buffer.
		InexorBuffer staging_buffer_for_texture;

		// Create a staging buffer for the texture.
		// This buffer is used as a transfer source for the buffer copy
		// TODO: Use generalized buffer creation manager?
		VkResult result = create_texture_buffer(texture, staging_buffer_for_texture, texture_memory_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		vulkan_error_check(result);

		// Copy memory to staging buffer.
		// Vma library already ensures that the memory is mapped for us!
		std::memcpy(staging_buffer_for_texture.allocation_info.pMappedData, pixels, static_cast<std::size_t>(texture_memory_size));

		// We now can discard the image data since we copied it already.
		stbi_image_free(pixels);

		result = create_texture_image(texture, texture_width, texture_height, format, VK_IMAGE_TILING_OPTIMAL, VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
		vulkan_error_check(result);


		transition_image_layout(texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		
		copy_buffer_to_image(staging_buffer_for_texture.buffer, texture->image, texture_width, texture_height);

		transition_image_layout(texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


		// Destroy staging buffer.
		vmaDestroyBuffer(vma_allocator, staging_buffer_for_texture.buffer, staging_buffer_for_texture.allocation);


		/// Create an image view so shaders can access this texture.
		create_texture_image_view(texture, format);

		// Create a texture sampler so shaders can access this texture.
		create_texture_sampler(texture);

		// Update the texture's descriptor.
		texture->update_descriptor();

		// TODO: Check if texture with this name does already exist.
		textures.insert({texture_name, texture});

		return VK_SUCCESS;
	}


	VkResult VulkanTextureManager::create_texture_sampler(std::shared_ptr<InexorTexture> texture)
	{
		VkSamplerCreateInfo sampler_create_info = {};

		sampler_create_info.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter    = VK_FILTER_LINEAR;
		sampler_create_info.minFilter    = VK_FILTER_LINEAR;
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
		sampler_create_info.maxAnisotropy    = 16;
		
		// The borderColor field specifies which color is returned when sampling beyond
		// the image with clamp to border addressing mode. It is possible to return black,
		// white or transparent in either float or int formats. You cannot specify an arbitrary color.
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// The unnormalizedCoordinates field specifies which coordinate system you
		// want to use to address texels in an image. If this field is VK_TRUE, then you
		// can simply use coordinates within the [0, texWidth) and [0, texHeight)
		// range. If it is VK_FALSE, then the texels are addressed using the [0, 1) range
		// on all axes. Real-world applications almost always use normalized coordinates,
		// because then it’s possible to use textures of varying resolutions with the exact
		// same coordinates.
		sampler_create_info.unnormalizedCoordinates = VK_FALSE;

		sampler_create_info.compareEnable = VK_FALSE;

		// TODO: Verify if we should use something else than this.
		sampler_create_info.compareOp     = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias    = 0.0f;
		sampler_create_info.minLod        = 0.0f;
		sampler_create_info.maxLod        = 0.0f;


		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceFeatures(graphics_card, &device_features);
		
		VkPhysicalDeviceProperties graphics_card_properties;
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);


		if(VK_TRUE == device_features.samplerAnisotropy)
		{
			// Anisotropic filtering is available.
			sampler_create_info.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
			sampler_create_info.anisotropyEnable = VK_TRUE;
		}
		else
		{
			// The device does not support anisotropic filtering
			sampler_create_info.maxAnisotropy = 1.0;
			sampler_create_info.anisotropyEnable = VK_FALSE;
		}

		VkResult result = vkCreateSampler(device, &sampler_create_info, nullptr, &texture->sampler);
		vulkan_error_check(result);

		// Give this texture sampler an appropriate name.
		const std::string texture_sampler_name = "Texture sampler for texture '" + texture->texture_name + "'.";

		dbg_marker_manager->set_object_name(device, (uint64_t)(texture->sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, texture_sampler_name.c_str());

		return VK_SUCCESS;
	}


	VkResult VulkanTextureManager::transition_image_layout(VkImage& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		// Start the recording of a command buffer.
		begin_single_time_commands();

		VkImageMemoryBarrier barrier = {};

		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout                       = old_layout;
		barrier.newLayout                       = new_layout;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.image                           = image;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if(VK_IMAGE_LAYOUT_UNDEFINED == old_layout && VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == new_layout)
		{
			// 
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == old_layout && VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == new_layout)
		{
			// 
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			
			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			spdlog::error("Error: unsupported layout transition!");
			return VK_ERROR_INITIALIZATION_FAILED; 
		}

		spdlog::debug("Recording pipeline barrier for image layer transition");

		// 
		vkCmdPipelineBarrier(data_transfer_command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		
		// End the recording of a command buffer.
		end_single_time_commands();

		return VK_SUCCESS;
	}
	
	
	VkResult VulkanTextureManager::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		// Start the recording of a command buffer.
		begin_single_time_commands();

		VkBufferImageCopy buffer_image_region = {};

		buffer_image_region.bufferOffset                    = 0;
		buffer_image_region.bufferRowLength                 = 0;
		buffer_image_region.bufferImageHeight               = 0;
		buffer_image_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_image_region.imageSubresource.mipLevel       = 0;
		buffer_image_region.imageSubresource.baseArrayLayer = 0;
		buffer_image_region.imageSubresource.layerCount     = 1;
		buffer_image_region.imageOffset                     = {0, 0, 0};
		buffer_image_region.imageExtent                     = {width, height, 1};

		vkCmdCopyBufferToImage(data_transfer_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &buffer_image_region);

		// End the recording of a command buffer.
		end_single_time_commands();

		return VK_SUCCESS;
	}
	
	
	VkImageView VulkanTextureManager::get_texture_view(const std::string& texture_name)
	{
		// TODO: Check if index exists.
		return textures[texture_name]->view;
	}
    
	
	VkSampler VulkanTextureManager::get_texture_sampler(const std::string& texture_name)
	{
		// TODO: Check if index exists.
		return textures[texture_name]->sampler;
	}


	void VulkanTextureManager::shutdown_textures()
	{
		std::unordered_map<std::string, std::shared_ptr<InexorTexture>>::iterator texture_iterator;
	
		// Iterate through all textures and destroy them.
		for(texture_iterator = textures.begin(); texture_iterator != textures.end(); texture_iterator++)
		{
			texture_iterator->second->destroy_texture(device, vma_allocator);
		}
		
		// Destroy command pool for texture data transfer.
		vkDestroyCommandPool(device, data_transfer_command_pool, nullptr);
	}


};
};
