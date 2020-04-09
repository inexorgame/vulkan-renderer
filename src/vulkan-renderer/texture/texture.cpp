#include "texture.hpp"


namespace inexor {
namespace vulkan_renderer {


	void InexorTexture::destroy_texture(const VkDevice& device, const VmaAllocator& vma_allocator)
	{
		vkDestroySampler(device, sampler, nullptr);

		vmaDestroyImage(vma_allocator, image, allocation);

		vkDestroyImageView(device, image_view, nullptr);

		// We don't need to destroy any buffers in here.

		file_name = "";

		name = "";

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
		descriptor.sampler = sampler;
		descriptor.imageView = image_view;
		descriptor.imageLayout = image_layout;
	}


};
};
