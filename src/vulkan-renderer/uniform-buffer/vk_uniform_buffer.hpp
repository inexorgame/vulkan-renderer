#pragma once

#include "../buffers/vk_buffer.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <mutex>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorUniformBuffer
	/// @brief A wrapper class for uniform buffers.
	/// Uniform buffers can be updated frequently, like every frame.
	struct InexorUniformBuffer
	{
		// We have to create every uniform buffer as often as there are images in the swapchain.
		std::vector<InexorBuffer> buffers;

		std::size_t images_in_swapchain = 0;
		
		/// @brief Initialises the uniform buffer.
		/// @param number_of_images_in_swapchain [in] The number of images in the swapchain.
		void setup(const std::size_t number_of_images_in_swapchain)
		{
			buffers.resize(number_of_images_in_swapchain);

			images_in_swapchain = number_of_images_in_swapchain;
		}


		/// @brief Updates a uniform buffer.
		/// @param data_source_address [in] The address of the source memory to copy.
		/// @param uniform_buffer_size [in] The size of the object to copy.
		void update_buffers(void* data_source_address, const std::size_t uniform_buffer_size)
		{
			for(std::size_t i=0; i<images_in_swapchain; i++)
			{				
				// Copy memory to uniform buffer!
				std::memcpy(buffers[i].allocation_info.pMappedData, data_source_address, uniform_buffer_size);
			}
		}


		void shutdown()
		{
			// TODO!
		}


	};


};
};
