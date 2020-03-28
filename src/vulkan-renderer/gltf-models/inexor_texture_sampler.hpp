#pragma once

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorTextureSampler
	struct InexorTextureSampler
	{
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressModeU;
		VkSamplerAddressMode addressModeV;
		VkSamplerAddressMode addressModeW;
	};

};
};
