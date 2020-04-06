#pragma once

#include "../descriptor-pool/InexorDescriptorPool.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>


namespace inexor {
namespace vulkan_renderer {


	class InexorDescriptorBundle
	{
		private:
		
			std::string internal_descriptor_set_name;

			std::shared_ptr<InexorDescriptorPool> descriptor_pool;


		public:


			// Force to use overloaded constructor!
			InexorDescriptorBundle() = delete;


			/// @brief Allow name and descriptor pool to be set in constructor only!
			InexorDescriptorBundle(const std::string& internal_descriptor_set_name,
			                       std::shared_ptr<InexorDescriptorPool> descriptor_pool)
			{
				this->internal_descriptor_set_name = internal_descriptor_set_name;
				this->descriptor_pool = descriptor_pool;
			}
			

			std::string get_name() const
			{
				return internal_descriptor_set_name;
			}


			VkDescriptorPool get_descriptor_pool() const
			{
				return descriptor_pool->pool;
			}

			VkDescriptorSetLayout descriptor_set_layout;

			std::vector<VkDescriptorSet> descriptor_sets;
			
			std::vector<VkWriteDescriptorSet> write_descriptor_sets;
			
			std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;


	};


};
};
