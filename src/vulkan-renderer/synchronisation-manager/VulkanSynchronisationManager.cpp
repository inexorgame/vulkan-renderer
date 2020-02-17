#include "VulkanSynchronisationManager.hpp"
using namespace std;


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
		
        // Use lock guard to ensure thread safety during write operations!
        std::lock_guard<std::mutex> lock(vulkan_synchronisation_manager_mutex);

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
        // Use lock guard to ensure thread safety during write operations!
        std::lock_guard<std::mutex> lock(vulkan_synchronisation_manager_mutex);

		// Create an iterator for the unordered map.
		std::unordered_map<std::string, VkSemaphore>::const_iterator semaphore_iterator = semaphores.begin();
 
		// Iterate over the unordered map.
		while(semaphore_iterator != semaphores.end())
		{
			cout << "Shutting down semaphore " << semaphore_iterator->first.c_str() << endl;


			// Destroy the semaphore.
			vkDestroySemaphore(vulkan_device, semaphore_iterator->second, nullptr);

			// Move on to the next Semaphore.
			semaphore_iterator++;
		}

		// Clear the unordered map!
		semaphores.clear();
	}

	
	bool VulkanSynchronisationManager::does_fence_exist(const std::string& fence_name) const
	{
		std::unordered_map<std::string, VkFence>::const_iterator fence_lookup = fences.find(fence_name);
		return fence_lookup != fences.end();

	}


	const std::optional<VkFence> VulkanSynchronisationManager::create_fence(const VkDevice& vulkan_device, const std::string& fence_name)
	{
		// First check if a Vulkan fence with this name already exists!
		if(does_fence_exist(fence_name))
		{
			std::string error_message = "Error: Vulkan fence with the name " + fence_name + " does already exist!";
			display_error_message(error_message);
			return std::nullopt;
		}

		VkFenceCreateInfo fence_create_info = {};
	
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext = nullptr;

		// Create this fence in a signaled state!
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// The new Vulkan fence which will be created.
		VkFence new_fence;

		VkResult result = vkCreateFence(vulkan_device, &fence_create_info, nullptr, &new_fence);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return std::nullopt;
		}
		
        // Use lock guard to ensure thread safety during write operations!
        std::lock_guard<std::mutex> lock(vulkan_synchronisation_manager_mutex);

		fences.insert({fence_name, new_fence});

		return new_fence;
	}

			
	const std::optional<VkFence> VulkanSynchronisationManager::get_fence(const std::string& fence_name) const
	{
		if(!does_fence_exist(fence_name))
		{
			std::string error_message = "Error: Vulkan fence with the name " + fence_name + " does not exists!";
			display_error_message(error_message);
			return std::nullopt;
		}

		// Return the requested semaphore.
		return fences.at(fence_name);
	}

			
	void VulkanSynchronisationManager::shutdown_fences(const VkDevice& vulkan_device)
	{
        // Use lock guard to ensure thread safety during write operations!
        std::lock_guard<std::mutex> lock(vulkan_synchronisation_manager_mutex);

		// Create an iterator for the unordered map.
		std::unordered_map<std::string, VkFence>::const_iterator fence_iterator = fences.begin();
 
		// Iterate over the unordered map.
		while(fence_iterator != fences.end())
		{
			cout << "Shutting down fence " << fence_iterator->first.c_str() << endl;
			
			// Destroy the fences.
			vkDestroyFence(vulkan_device, fence_iterator->second, nullptr);

			// Move on to the next Semaphore.
			fence_iterator++;
		}

		// Clear the unordered map!
		fences.clear();
	}


};
};
