#pragma once

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT.
#include "vma/vma_usage.h"

// JSON for modern C++11 library.
// https://github.com/nlohmann/json
/// License: MIT.
#include "nlohmann/json.hpp"
#define TINYGLTF_NO_INCLUDE_JSON

// Header only C++ tiny glTF library(loader/saver).
// https://github.com/syoyo/tinygltf
// License: MIT.
#include "tiny_gltf/tiny_gltf.h"

#include "vulkan-renderer/gpu-memory-buffer/gpu_memory_buffer.hpp"
#include "vulkan-renderer/texture/texture.hpp"
#include "vulkan-renderer/class-templates/manager_template.hpp"
#include "vulkan-renderer/debug-marker/debug_marker_manager.hpp"
#include "vulkan-renderer/command-buffer-recording/single_time_command_buffer.hpp"
#include "vulkan-renderer/error-handling/error_handling.hpp"

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include <string>
#include <memory>


namespace inexor
{
	namespace vulkan_renderer
	{


		// TODO: 2D textures, 3D textures and cube maps.
		// TODO: Scan asset directory automatically.
		// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

		/// @class VulkanTextureManager
		/// @brief A manager class for textures.
		/// @note We do not support linear tiled textures because it is not advisable to do so!
		class VulkanTextureManager : public ManagerClassTemplate<InexorTexture>,
			public SingleTimeCommandBufferRecorder
		{
			private:

			bool texture_manager_initialised = false;

			VmaAllocator vma_allocator;

			VkPhysicalDevice graphics_card = VK_NULL_HANDLE;

			uint32_t transfer_queue_family_index = 0;


			public:

			VulkanTextureManager() = default;

			~VulkanTextureManager() = default;


			private:

			/// @brief Creates the texture manager command pool which is neccesary for copying data to GPU memory.
			VkResult create_texture_manager_command_pool();


			/// @brief Allocates memory for a texture buffer of given size using Vulkan Memory Allocator library (VMA).
			/// @param texture [in] The texture for which a buffer will be created.
			/// @param buffer_object [in] The Inexor buffer object which will be created.
			/// @param buffer_size [in] The size of the buffer.
			/// @param buffer_usage [in] The buffer usage flags.
			/// @param memory_usage [in] The VMA memory usage flags.
			VkResult create_texture_buffer(std::shared_ptr<InexorTexture> texture, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage);


			/// @brief Creates a texture image.
			/// @param texture [in] The texture for which an image will be created.
			/// @param format [in] The image format.
			/// @param tiling [in] The image tiling.
			/// @param buffer_usage [in] The buffer usage flags.
			/// @param memory_usage [in] The VMA memory usage flags.
			/// @param image_usage_flags [in] The image usage flags.
			VkResult create_texture_image(std::shared_ptr<InexorTexture> texture, const VkFormat& format, const VkImageTiling& tiling, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage, const VkImageUsageFlags& image_usage_flags);


			/// @brief Creates a texture image view.
			/// @param texture [in] The texture for which a buffer will be created.
			/// @param format [in] The image format.
			VkResult create_texture_image_view(std::shared_ptr<InexorTexture> texture, const VkFormat& format);


			/// @brief Copies an image to a buffer
			/// @param buffer [in] The target buffer.
			/// @param image [in] The image.
			/// @param width [in] The width of the image.
			/// @param width [in] The height of the image.
			VkResult copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


			/// @brief 
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult transition_image_layout(VkImage& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);


			/// @brief Creates a texture sampler so shaders can access image data.
			/// @param texture [in] The texture for which a texture sampler will be created.
			VkResult create_texture_sampler(std::shared_ptr<InexorTexture> texture);



			public:

			/// @brief Initialises texture manager by passing some pointers that we need.
			/// @param device [in] The Vulkan device.
			/// @param graphics_card [in] The graphics card.
			/// @param debug_marker_manager [in] The Vulkan debug marker pointer (only available when VK_EXT_debug_marker is available and enabled!)
			/// @param vma_allocator [in] An instance of the Vulkan memory allocator library.
			/// @param transfer_queue_family_index [in] The queue family index of the data transfer queue (could be distinct queue or graphics queue).
			/// @param data_transfer_queue [in] The data transfer queue (could be distinct queue or graphics queue).
			VkResult initialise(const VkDevice& device, const VkPhysicalDevice& graphics_card, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue);


			/// @brief Creates a texture from a file of supported format.
			/// @note Since we are using STB library, we can load any image format which is supported by it: JPG, PNG, BMP, TGA (and more).
			/// @param internal_texture_name [in] The internal name which will be used inside the engine.
			/// @param texture_file_name [in] The name of the texture file.
			/// @param output_texture [out] The texture which will be created. It can be nullptr if creating the texture fails.
			VkResult create_texture_from_file(const std::string& internal_texture_name, const std::string& texture_file_name, std::shared_ptr<InexorTexture> output_texture);


			/// @brief Create a texture from an unsigned char buffer.
			/// @param internal_texture_name [in] The internal name which will be used inside the engine.
			/// @param texture_memory [in] The Inexor texture buffer which will be created for this texture.
			/// @param texture_memory_size [in] The size of the texture's memory.
			/// @param output_texture [out] The texture which will be created. It can be nullptr if creating the texture fails.
			VkResult create_texture_from_memory(const std::string& internal_texture_name, void* texture_memory, const VkDeviceSize& texture_memory_size, std::shared_ptr<InexorTexture> output_texture);


			/// @brief Creates a new texture from a glTF 2.0 file.
			/// @param internal_texture_name [in] The internal name which will be used inside the engine.
			/// @param gltf_image [in] The glTF 2.0 image.
			/// @param output_texture [out] The texture which will be created. It can be nullptr if creating the texture fails.
			VkResult create_texture_from_glTF2_image(const std::string& internal_texture_name, tinygltf::Image& gltf_image, std::shared_ptr<InexorTexture> output_texture);


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
