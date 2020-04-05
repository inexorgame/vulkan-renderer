#pragma once

#include "../buffers/vk_buffer.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"
#include "../class-templates/manager_template.hpp"
#include "../uniform-buffer/vk_uniform_buffer.hpp"

#define GLM_FORCE_RADIANS

// This will force GLM to use a version of vec2 and mat4 that has the alignment requirements already specified for us.
/// @warning Unfortunately this method can break down if you start using nested structures.
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include <string>
#include <unordered_map>
#include <cassert>


namespace inexor {
namespace vulkan_renderer {

	
	class VulkanUniformBufferManager : public ManagerClassTemplate<InexorUniformBuffer>
	{
		private:

			VkDevice device = VK_NULL_HANDLE;

			bool uniform_buffer_initialised = false;

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			VmaAllocator vma_allocator;

			std::size_t number_of_images_in_swapchain = 0;


		private:

			/// @brief Creates a new uniform buffer.
			/// @param internal_buffer_name [in] The engine-internal name of the uniform buffer.
			/// @param buffer_size [in] The size of the buffer.
			/// @param buffer_object [out] The Inexor buffer object.
			VkResult create_buffer(std::string& internal_buffer_name,
			                       const VkDeviceSize& buffer_size,
								   std::shared_ptr<InexorUniformBuffer>& buffer_object);


			/// @brief Destroys all uniform buffers.
			VkResult destroy_uniform_buffers();


		public:
			
			VulkanUniformBufferManager() = default;


			~VulkanUniformBufferManager() = default;


			/// @brief Initialises the buffer manager.
			/// @param device [in] The Vulkan device.
			/// @param vma_allocator [in] The Vulkam memory allocator handle.
			/// @param debug_marker_manager [in] A pointer to the debug marker manager.
			VkResult initialise(const VkDevice& device,
			                    const VmaAllocator& vma_allocator,
								const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager);
			

			/// @brief Creates a new uniform buffer.
			/// @param internal_uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param uniform_buffer_size [in] The size of the uniform buffer.
			/// @param uniform_buffer_output [out] The uniform buffers which will be created.
			VkResult create_uniform_buffer(const std::string& internal_uniform_buffer_name,
			                               const VkDeviceSize& uniform_buffer_size,
										   std::shared_ptr<InexorUniformBuffer>& uniform_buffer);


			/// @TODO: Store the shared pointer when using this API so we don't have to lookup the unordered_map every time.
			/// Then, implement an update method which updates an entry in the unordered_map by value.

			/// @brief Updates a certain uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param uniform_buffer_new_data_source [in] A pointer to the new uniform buffer data.
			/// @param uniform_buffer_size [in] The size of the uniform buffer to copy from.
			/// @warning The size of the source memory must not be greater than the size of the target memory!
			VkResult update_uniform_buffer(const std::string& internal_uniform_buffer_name,
			                               void* uniform_buffer_new_data_source,
										   const std::size_t uniform_buffer_size);


			/// @brief Updates a certain uniform buffer by memory pointer.
			/// @param uniform_buffer [in] A reference to the uniform buffer to update.
			/// @param uniform_buffer_new_data_source [in] A pointer to the new uniform buffer data.
			/// @param uniform_buffer_size [in] The size of the uniform buffer to copy from.
			/// @warning The size of the source memory must not be greater than the size of the target memory!
			VkResult update_uniform_buffer(std::shared_ptr<InexorUniformBuffer>& uniform_buffer,
			                               void* uniform_buffer_new_data_source,
										   const std::size_t uniform_buffer_size);



			/// @brief Destroys all uniform buffers.
			VkResult shutdown_uniform_buffers();


	};


};
};
