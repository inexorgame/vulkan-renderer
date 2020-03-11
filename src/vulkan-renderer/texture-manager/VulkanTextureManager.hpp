#pragma once

#include <vulkan/vulkan.h>

#include "../buffers/InexorBuffer.hpp"
#include "../debug-marker/VulkanDebugMarkerManager.hpp"
#include "../texture/InexorTexture.hpp"

/// Vulkan Memory Allocator library:
/// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include <spdlog/spdlog.h>

#include <string>
#include <memory>


namespace inexor {
namespace vulkan_renderer {

	
	// TODO: 2D textures, 3D textures and cube maps.
	
	/// @class VulkanTextureManager
	/// @brief A manager class for textures.
	class VulkanTextureManager
	{
		private:
		
			// The textures.
			std::unordered_map<std::string, std::shared_ptr<InexorTexture>> textures;

		private:
			
			// The debug marker manager.
			std::shared_ptr<VulkanDebugMarkerManager> dbg_marker_manager;
			
			// The Vulkan Memory Allocator handle.
			VmaAllocator vma_allocator;

			// The command pool for data transfer.
			VkCommandPool data_transfer_command_pool = VK_NULL_HANDLE;

			// The command buffer for data transfer to GPU memory.
			VkCommandBuffer data_transfer_command_buffer = VK_NULL_HANDLE;

			// The data transfer queue.
			VkQueue data_transfer_queue = VK_NULL_HANDLE;

			// The Vulkan device.
			VkDevice device = VK_NULL_HANDLE;

			// The graphics card.
			VkPhysicalDevice graphics_card = VK_NULL_HANDLE;


		public:

			VulkanTextureManager();
			
			~VulkanTextureManager();

		
		private:

			/// @brief 
			VkResult begin_single_time_commands();

			
			/// @brief 
			VkResult end_single_time_commands();


			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			VkResult create_texture_buffer(const std::string& texture_name, InexorBuffer& buffer_object, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage);
			
			
			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult create_texture_image(std::shared_ptr<InexorTexture> texture, const uint32_t& texture_width, const uint32_t& texture_height, const VkFormat& format, const VkImageTiling& tiling, const VmaMemoryUsage& memory_usage, const VkBufferUsageFlags& buffer_usage, const VkImageUsageFlags& image_usage_flags);
			
			
			/// @brief 
			/// @param 
			/// @param 
			VkResult create_texture_image_view(std::shared_ptr<InexorTexture> texture, const VkFormat& format);


			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult copy_buffer_to_image(VkBuffer buffer, VkImage& image, uint32_t width, uint32_t height);


			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult transition_image_layout(VkImage& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);


			/// @brief Creates a texture sampler for shader access to image data.
			/// @param texture The Inexor texture buffer.
			VkResult create_texture_sampler(std::shared_ptr<InexorTexture> texture);



		protected:


			/// @brief Initialises texture manager by passing some pointers that we need.
			/// @param device The Vulkan device.
			/// @param debug_marker_manager_instance The Vulkan debug marker pointer (only available when VK_EXT_debug_marker is available and enabled!)
			/// @param vma_allocator An instance of the Vulkan memory allocator library.
			/// @param transfer_queue_family_index The queue family index of the data transfer queue (could be distinct queue or graphics queue).
			/// @param data_transfer_queue The data transfer queue (could be distinct queue or graphics queue).
			VkResult initialise(const VkDevice& device, const VkPhysicalDevice& graphics_card, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,  const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue);


			// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.


			/// @brief Creates a texture from a file of supported format.
			/// @note Since we are using STB, we can load any image format which is supported by it: JPG, PNG, BMP, TGA (and more).
			/// @param file_name The name of the texture file.
			/// @param texture The Inexor texture buffer which will be created for this texture.
			VkResult create_texture_from_file(const std::string& texture_name, const std::string& file_name, std::shared_ptr<InexorTexture> texture);


			/// @brief Returns the view of a certain texture by name.
			/// @param texture_name The name of the texture.
			VkImageView get_texture_view(const std::string& texture_name);
			

			/// @brief Returns the sampler of a certain texture by name.
			/// @param texture_name The name of the texture.
			VkSampler get_texture_sampler(const std::string& texture_name);


			/// @brief Destroys all textures.
			void shutdown_textures();


	};


};
};
