#pragma once

#include "../buffers/vk_buffer.hpp"

#include <glm/glm.hpp>

#include <vector>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorUniformBuffer
	/// @brief A wrapper class for uniform buffers.
	/// Uniform buffers can be updated frequently, like every frame.
	struct InexorUniformBuffer : public InexorBuffer
	{

		/// @brief Updates the uniform buffer.
		/// @param data_source_address [in] The address of the source memory to copy.
		/// @param uniform_buffer_size [in] The size of the object to copy.
		void update_buffer(void* data_source_address, const std::size_t uniform_buffer_size)
		{
			assert(data_source_address);
			assert(allocation_info.pMappedData);
			
			// TODO: Add mutex!
			std::memcpy(allocation_info.pMappedData, data_source_address, uniform_buffer_size);
		}


		/// 
		VkBuffer get_buffer()
		{
			// TODO: Add mutex!
			return buffer;
		}


	};


};
};
