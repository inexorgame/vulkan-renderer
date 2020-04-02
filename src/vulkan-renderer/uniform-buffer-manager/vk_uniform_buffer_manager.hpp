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

#include <cassert>

#include "../buffers/vk_buffer.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"
#include "../class-templates/manager_template.hpp"
#include "../uniform-buffer/vk_uniform_buffer.hpp"
#include "../uniform-buffer/vk_uniform_buffer_group.hpp"


namespace inexor {
namespace vulkan_renderer {

	

	/// @brief A manager class for uniform buffers.
	class VulkanUniformBufferManager : public ManagerClassTemplate<InexorUniformBuffer>
	{
		private:

			VkDevice device = VK_NULL_HANDLE;

			bool uniform_buffer_initialised = false;

			std::mutex uniform_buffer_manager_mutex;

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			VmaAllocator vma_allocator;

			std::size_t number_of_images_in_swapchain = 0;


		private:

			/// @brief Creates a new uniform buffer.
			/// @param internal_buffer_name [in] The engine-internal name of the uniform buffer.
			/// @param buffer_object [out] The Inexor buffer object.
			/// @param buffer_size [in] The size of the buffer.
			VkResult create_buffer(std::string& internal_buffer_name, std::shared_ptr<InexorBuffer> buffer_object, const VkDeviceSize& buffer_size);


		public:
			
			VulkanUniformBufferManager();

			~VulkanUniformBufferManager();


			/// @brief Initialises the buffer manager.
			/// @param device [in] The Vulkan device
			/// @param number_of_images_in_swapchain [in] The number of images in swapchain.
			/// @param debug_marker_manager [in] A pointer to the debug marker manager.
			/// @param vma_allocator [in] The Vulkam memory allocator handle.
			VkResult initialise(const VkDevice& device, const std::size_t number_of_images_in_swapchain, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, VmaAllocator& vma_allocator);
			

			/// @brief Creates a new uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param uniform_buffer_size [in] The size of the uniform buffer.
			/// @param uniform_buffer_output [out] The uniform buffers which was created.
			/// @note We have to create every uniform buffer as often as there are images in the swapchain!
			/// @TODO Refactor this so we don't store output pointer if not desired.
			VkResult create_uniform_buffer(const std::string& internal_uniform_buffer_name, const VkDeviceSize& uniform_buffer_size, std::shared_ptr<InexorUniformBuffer> uniform_buffer_output);


			/// @brief Creates multiple uniform buffers of the same type. Each uniform bufer has the same size.
			/// @param internal_uniform_buffers_prefix [in] The prefix of the name of every uniform buffer.
			/// @param uniform_buffer_size [int] The size of every uniform buffer.
			/// @param uniform_buffers_output [out] A vector to the uniform buffers which were created.
			VkResult create_multiple_uniform_buffers(const std::string& internal_uniform_buffers_prefix, const VkDeviceSize& uniform_buffer_size, std::size_t number_of_buffers_to_create, std::vector<std::shared_ptr<InexorUniformBuffer>>& uniform_buffers_output);


			/// @brief Creates a uniform buffer for every image in swapchain.
			/// @param internal_uniform_buffers_prefix [in] The prefix of the name of every uniform buffer.
			/// @param uniform_buffer_size [int] The size of every uniform buffer.
			/// @param uniform_buffers_output [out] A vector to the uniform buffers which were created.
			VkResult create_uniform_buffer_for_every_image_in_swapchain(const std::string& internal_uniform_buffers_prefix, const VkDeviceSize& uniform_buffer_size, std::vector<std::shared_ptr<InexorUniformBuffer>>& uniform_buffers_output);


			/// @brief Returns a uniform buffer by name (key).
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			std::optional<std::shared_ptr<InexorUniformBuffer>> get_uniform_buffer(const std::string& internal_uniform_buffer_name);


			/// @brief Destroys a uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			VkResult destroy_uniform_buffer(const std::string& internal_uniform_buffer_name);


			/// @brief Updates a uniform buffer.
			/// @param uniform_buffer_name [in] The internal name of the uniform buffer.
			/// @param current_image_index [in] The current image index.
			/// @param uniform_buffer_data_source [in] A pointer to the uniform buffer data.
			/// @param uniform_buffer_size [in] The size of the uniform buffer.
			VkResult update_uniform_buffer(const std::string& internal_uniform_buffer_name, const std::size_t current_image_index, void* uniform_buffer_data_source, const std::size_t uniform_buffer_size);


			/// @brief Shutdown all uniform buffers.
			VkResult shutdown_uniform_buffers();


	};


};
};
