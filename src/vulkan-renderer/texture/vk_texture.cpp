#include "vk_texture.hpp"


namespace inexor {
namespace vulkan_renderer {


	InexorTexture::InexorTexture()
	{
	}

	
	InexorTexture::~InexorTexture()
	{
	}


	void InexorTexture::destroy_texture(const VkDevice& device, const VmaAllocator& vma_allocator)
	{
		vkDestroySampler(device, sampler, nullptr);

		vmaDestroyImage(vma_allocator, image, allocation);

		vkDestroyImageView(device, image_view, nullptr);

		// We don't need to destroy any buffers in here.

		texture_file_name = "";

		texture_name = "";

		image = VK_NULL_HANDLE;

		image_view = VK_NULL_HANDLE;

		sampler = VK_NULL_HANDLE;

		uint32_t layer_count = 0;

		uint32_t mip_levels = 0;

		uint32_t texture_width  = 0;

		uint32_t texture_height = 0;
	}


	void InexorTexture::update_descriptor()
	{
		descriptor_image_info.sampler = sampler;
		descriptor_image_info.imageView = image_view;
		descriptor_image_info.imageLayout = image_layout;
	}


};
};
