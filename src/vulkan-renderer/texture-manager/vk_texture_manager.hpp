#pragma once

#include "../buffers/vk_buffer.hpp"
#include "../texture/vk_texture.hpp"
#include "../class-templates/manager_template.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"
#include "../command-buffer-recording/vk_single_time_command_buffer.hpp"
#include "../error-handling/vk_error_handling.hpp"


// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include "../../third_party/vma/vk_mem_alloc.h"


// JSON for modern C++11 library.
// https://github.com/nlohmann/json
/// License: MIT
#include "nlohmann/json.hpp"
#define TINYGLTF_NO_INCLUDE_JSON

// Header only C++ tiny glTF library(loader/saver).
// https://github.com/syoyo/tinygltf
// License: MIT
#include "../../third_party/tiny_gltf/tiny_gltf.h"

// stb single-file public domain libraries for C/C++
// https://github.com/nothings/stb
// License: Public Domain
#include "stb_image.h"

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include <string>
#include <memory>


namespace inexor {
namespace vulkan_renderer {

	
	// TODO: 2D textures, 3D textures and cube maps.
	// TODO: Scan asset directory automatically.
	
	/// @class VulkanTextureManager
	/// @brief A manager class for textures.
	// Note: Inexor vulkan-renderer does not intend to support linear tiled textures because it is not advisable to do so!
	// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.
	class VulkanTextureManager : public ManagerClassTemplate<InexorTexture>,
                                 public SingleTimeCommandBufferRecorder
	{
		private:
			
			bool texture_manager_initialised = false;
			
			VmaAllocator vma_allocator;

			VkPhysicalDevice graphics_card = VK_NULL_HANDLE;


		public:

			VulkanTextureManager();
			
			~VulkanTextureManager();

		
		private:

			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			VkResult create_texture_buffer(std::shared_ptr<InexorTexture> texture, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage);
			
			
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
			VkResult copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult transition_image_layout(VkImage& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);


			/// @brief Creates a texture sampler so shaders can access image data.
			/// @param texture [in] The Inexor texture buffer.
			VkResult create_texture_sampler(std::shared_ptr<InexorTexture> texture);


		public:

			/// @brief Initialises texture manager by passing some pointers that we need.
			/// @param device The Vulkan device.
			/// @param debug_marker_manager_instance The Vulkan debug marker pointer (only available when VK_EXT_debug_marker is available and enabled!)
			/// @param vma_allocator An instance of the Vulkan memory allocator library.
			/// @param transfer_queue_family_index The queue family index of the data transfer queue (could be distinct queue or graphics queue).
			/// @param data_transfer_queue The data transfer queue (could be distinct queue or graphics queue).
			VkResult initialise(const VkDevice& device, const VkPhysicalDevice& graphics_card, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,  const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue);


			/// @brief Creates a texture from a file of supported format.
			/// @note Since we are using STB library, we can load any image format which is supported by it: JPG, PNG, BMP, TGA (and more).
			/// @param internal_texture_name [in] The name of the texture file.
			/// @param texture_file_name [in] The name of the texture file.
			/// @param texture [in] The Inexor texture buffer which will be created for this texture.
			VkResult create_texture_from_file(const std::string& internal_texture_name, const std::string& texture_file_name, std::shared_ptr<InexorTexture> output_texture);


			/// @brief Creates a new texture from a glTF 2.0 file.
			/// @param internal_texture_name [in] The name of the texture file.
			/// @param gltf_image [in] The glTF 2.0 image.
			VkResult create_texture_from_glTF2_image(const std::string& internal_texture_name, tinygltf::Image &gltf_image, std::shared_ptr<InexorTexture> output_texture);
		

			/// @brief Returns a certain texture by internal name (key).
			/// @param internal_texture_name [in] The internal name of the texture
			/// @return A std::optional shared pointer to the texture instance.
			std::optional<std::shared_ptr<InexorTexture>> get_texture(const std::string& internal_texture_name);

			
			/// @brief Returns the view of a certain texture by name.
			/// @param internal_texture_name [in] The name of the texture.
			std::optional<VkImageView> get_texture_view(const std::string& internal_texture_name);
			

			/// @brief Returns the sampler of a certain texture by name.
			/// @param internal_texture_name [in] The name of the texture.
			std::optional<VkSampler> get_texture_sampler(const std::string& internal_texture_name);


			/// @brief Destroys all textures.
			void shutdown_textures();


	};


};
};
