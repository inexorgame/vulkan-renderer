#include "InexorTexture.hpp"


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

		vkDestroyImage(device, image, nullptr);

		vkDestroyImageView(device, view, nullptr);

		vmaDestroyBuffer(vma_allocator, buffer, allocation);

		file_name = "";

		texture_name = "";

		image = VK_NULL_HANDLE;

		view = VK_NULL_HANDLE;

		sampler = VK_NULL_HANDLE;

		uint32_t layer_count = 0;

		uint32_t mip_levels = 0;

		uint32_t texture_width  = 0;

		uint32_t texture_height = 0;
	}


	void InexorTexture::update_descriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = layout;
	}


};
};
