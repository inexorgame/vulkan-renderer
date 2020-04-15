#pragma once

#include "vulkan-renderer/image-buffer/image_buffer.hpp"


namespace inexor {
namespace vulkan_renderer {


	/// 
	/// 
	/// 
	struct InexorMSAATarget
	{
		// The color buffer.
		InexorImageBuffer color;

		// The depth buffer.
		InexorImageBuffer depth;

	};


};
};
