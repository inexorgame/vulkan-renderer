#pragma once

#include "../buffers/vk_buffer.hpp"

#include <vulkan/vulkan.h>

#include <string>


namespace inexor {
namespace vulkan_renderer {

	
	class InexorTexture
	{
		public:
			
			InexorTexture();
			
			~InexorTexture();


		public:

			std::string texture_name = "";

			std::string texture_file_name = "";
			
			VkImageViewCreateInfo view_create_info = {};

			VkImageCreateInfo image_create_info = {};

			VmaAllocation allocation = VK_NULL_HANDLE;

			VmaAllocationInfo allocation_info = {};

			VkBufferCreateInfo create_info = {};
				
			VmaAllocationCreateInfo allocation_create_info = {};	

			VkImage image = {};

			VkImageView image_view = {};

			VkImageLayout image_layout = {};

			VkDescriptorImageInfo descriptor_image_info = {};

			VkSampler sampler = {};

			uint32_t layer_count = 0;

			uint32_t mip_levels = 0;

			uint32_t texture_width = 0;

			uint32_t texture_height = 0;


		public:
			
			/// @brief Destroys a texture.
			/// @param device [in] The Vulkan device.
			/// @param vma_allocator [in] The Vulkan memory allocator handle.
			void destroy_texture(const VkDevice& device, const VmaAllocator& vma_allocator);
			

			/// @brief Updates the texture sampler's descriptor.
			void update_descriptor();

	};

};
};
