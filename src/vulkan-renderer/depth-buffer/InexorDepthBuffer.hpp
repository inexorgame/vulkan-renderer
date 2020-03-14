#pragma once

#include "../buffers/InexorBuffer.hpp"


namespace inexor {
namespace vulkan_renderer {


	/// 
	class InexorDepthBuffer : public InexorBuffer
	{
		public:

			InexorDepthBuffer()
			{
			}
			
			~InexorDepthBuffer()
			{
			}


			// The actual depth buffer image.
			VkImage image;

			// Depth buffer view.
			VkImageView image_view;
			
			// The depth buffer's format.
			std::optional<VkFormat> format;

			// The data of the depth buffer.
			InexorBuffer buffer;

	};

};
};
