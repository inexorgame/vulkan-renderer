#include "VulkanSynchronisationManager.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanSynchronisationManager::VulkanSynchronisationManager()
	{
	}
	
	
	VulkanSynchronisationManager::~VulkanSynchronisationManager()
	{
	}
	

	bool VulkanSynchronisationManager::does_semaphore_exist(const std::string&semaphore_name) const
	{
		std::unordered_map<std::string, VkSemaphore>::const_iterator semaphore_lookup = semaphores.find(semaphore_name);

		return semaphore_lookup != semaphores.end();
	}


	const std::optional<VkSemaphore> VulkanSynchronisationManager::create_semaphore(const VkDevice& vulkan_device, const std::string& semaphore_name)
	{
		// First check if a Vulkan semaphore with this name already exists!
		if(does_semaphore_exist(semaphore_name))
		{
			std::string error_message = "Error: Vulkan semaphore with the name " + semaphore_name + " does already exist!";
			display_error_message(error_message);
			return std::nullopt;
		}

		VkSemaphoreCreateInfo semaphore_create_info = {};

		// So far, there is nothing to fill into this structure.
		// This may change in the future!
		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSemaphoreCreateInfo.html
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext = nullptr;
		semaphore_create_info.flags = 0;
		
		// The new Vulkan semaphore which will be created.
		VkSemaphore new_semaphore;

		VkResult result = vkCreateSemaphore(vulkan_device, &semaphore_create_info, nullptr, &new_semaphore);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return std::nullopt;
		}

		semaphores.insert({semaphore_name, new_semaphore});

		return new_semaphore;
	}


	const std::optional<VkSemaphore> VulkanSynchronisationManager::get_semaphore(const std::string& semaphore_name) const
	{
		if(!does_semaphore_exist(semaphore_name))
		{
			std::string error_message = "Error: Vulkan semaphore with the name " + semaphore_name + " does not exists!";
			display_error_message(error_message);
			return std::nullopt;
		}

		// Return the requested semaphore.
		return semaphores.at(semaphore_name);
	}


	void VulkanSynchronisationManager::shutdown_semaphores(const VkDevice& vulkan_device)
	{
		// Create an iterator for the unordered map.
		std::unordered_map<std::string, VkSemaphore>::const_iterator sepahore_iterator = semaphores.begin();
 
		// Iterate over the unordered map.
		while(sepahore_iterator != semaphores.end())
		{
			// Destroy the semaphore.
			vkDestroySemaphore(vulkan_device, sepahore_iterator->second, nullptr);

			// Move on to the next Semaphore.
			sepahore_iterator++;
		}

		// Clear the unordered map!
		semaphores.clear();
	}


};
};
