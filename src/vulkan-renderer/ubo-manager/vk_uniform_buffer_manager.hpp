#pragma once

#define GLM_FORCE_RADIANS

// This will force GLM to use a version of vec2 and mat4 that has the alignment requirements already specified for us.
/// @warning Unfortunately this method can break down if you start using nested structures.
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include <string>
#include <unordered_map>

#include <assert.h>

#include "../buffers/vk_buffer.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"
#include "../class-templates/manager_template.hpp"
#include "../uniform-buffer/vk_uniform_buffer.hpp"


namespace inexor {
namespace vulkan_renderer {

	

	/// @brief A manager class for uniform buffers.
	class VulkanUniformBufferManager : public ManagerClassTemplate<InexorUniformBuffer>
	{
		private:

			VkDevice device;

			// The mutex of this class.
			std::mutex uniform_buffer_manager_mutex;

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			/// Vulkan Memory Allocator.
			/// Vulkan requires you to manage video memory for every type of resource like textures or vertex buffers manually.
			/// To avoid having to do the memory management explicitely, we will use the famous Vulkan memory allocator library by AMD.
			/// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
			VmaAllocator vma_allocator;


		private:

			/// @brief Creates a new uniform buffer.
			/// @param internal_buffer_name [in] The engine-internal name of the uniform buffer.
			/// @param buffer_object [out] The Inexor buffer object.
			/// @param buffer_size [in] The size of the buffer.
			VkResult create_buffer(std::string& internal_buffer_name, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size);


		public:
			
			VulkanUniformBufferManager();

			~VulkanUniformBufferManager();


		protected:


			/// @brief Initialises the buffer manager.
			/// @param device [in] The Vulkan device
			/// @param debug_marker_manager [in] A pointer to the debug marker manager.
			/// @param vma_allocator [in] The Vulkam memory allocator handle.
			VkResult initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, VmaAllocator& vma_allocator);
			

			/// @brief Creates a new uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param uniform_buffer_size [in] The size of the uniform buffer.
			/// @param number_of_images_in_swapchain [in] The number of images in the swapchain.
			/// @note We have to create every uniform buffer as often as there are images in the swapchain!
			VkResult create_uniform_buffer(const std::string& uniform_buffer_name, const VkDeviceSize& uniform_buffer_size, const std::size_t number_of_images_in_swapchain);

			
			/// @brief Returns a uniform buffer by name (key).
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			std::optional<std::shared_ptr<InexorUniformBuffer>> get_uniform_buffer(const std::string& uniform_buffer_name);


			/// @brief Destroys a uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			VkResult destroy_uniform_buffer(const std::string& uniform_buffer_name);


			/// @brief Updates a uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param current_image_index [in] The current image index.
			/// @param uniform_buffer_data_source [in] A pointer to the uniform buffer data.
			/// @param uniform_buffer_size [in] The size of the uniform buffer.
			VkResult update_uniform_buffer(const std::string& uniform_buffer_name, const std::size_t current_image_index, void* uniform_buffer_data_source, const std::size_t uniform_buffer_size);


			/// @brief Shutdown all uniform buffers.
			VkResult shutdown_uniform_buffers();


	};


};
};
