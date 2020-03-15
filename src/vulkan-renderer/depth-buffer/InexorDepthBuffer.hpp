#pragma once


namespace inexor {
namespace vulkan_renderer {


	/// 
	class InexorDepthBuffer
	{
		public:

			InexorDepthBuffer()
			{
			}
			
			~InexorDepthBuffer()
			{
			}


			// The VMA allocation.
			VmaAllocation allocation = VK_NULL_HANDLE;
			
			// The structure describing the VMA allocation.
			VmaAllocationCreateInfo allocation_create_info = {};	
			
			// The actual depth buffer image.
			VkImage image;

			// Depth buffer view.
			VkImageView image_view;
			
			// The depth buffer's format.
			std::optional<VkFormat> format;

	};

};
};
