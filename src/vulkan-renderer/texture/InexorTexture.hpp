#pragma once

#include <vulkan/vulkan.h>

#include "../buffers/InexorBuffer.hpp"

#include <string>


namespace inexor {
namespace vulkan_renderer {

	
	/// @class InexorTextureBuffer
	/// @brief A structure for textures.
	/// @note The texture's memory will be managed by InexorBuffer.
	class InexorTexture : public InexorBuffer
	{
		public:
			
			InexorTexture();
			
			~InexorTexture();


		public:

			// TODO: Store duplicated of device-related data in here?

			// The internal name of the texture which will be displayed to the user.
			std::string texture_name = "";

			// The file name of the texture.
			std::string file_name = "";

			// The texture's image buffer.
			VkImage image;

			// Images are accessed through views.
			VkImageView view;

			// The memory layout of the texture.
			VkImageLayout layout;

			// The image descriptor.
			VkDescriptorImageInfo descriptor;

			// The image sampler for shaders.
			VkSampler sampler;

			// The number of layers.
			uint32_t layer_count = 0;

			// Mipmap levels.
			uint32_t mip_levels = 0;

			// The width of the texture.
			uint32_t texture_width = 0;

			// The height of the texture.
			uint32_t texture_height = 0;


		public:
			
			/// @brief Destroys a texture.
			/// @param device The Vulkan device.
			/// @param vma_allocator The Vulkan memory allocator handle.
			void destroy_texture(const VkDevice& device, const VmaAllocator& vma_allocator);
			
			/// 
			void update_descriptor();

	};

};
};
