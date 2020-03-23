#include "vk_fence_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanFenceManager::VulkanFenceManager()
	{
	}
	
	
	VulkanFenceManager::~VulkanFenceManager()
	{
	}

	
	VkResult VulkanFenceManager::initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager)
	{
		assert(device);
		assert(debug_marker_manager);

		spdlog::debug("Initialising semaphore manager.");

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;

		fence_manager_initialised = true;

		return VK_SUCCESS;
	}


	bool VulkanFenceManager::does_fence_exist(const std::string& fence_name)
	{
		assert(fence_manager_initialised);
		assert(fence_name.length()>0);
		
		return does_key_exist(fence_name);
	}


	std::optional<std::shared_ptr<VkFence>> VulkanFenceManager::create_fence(const std::string& fence_name, bool create_as_signaled)
	{
		assert(device);
		assert(fence_name.length()>0);
		assert(fence_manager_initialised);

		// First check if a Vulkan fence with this name already exists!
		if(does_fence_exist(fence_name))
		{
			spdlog::error("Vulkan fence '{}' already exists!", fence_name);
			return std::nullopt;
		}

		VkFenceCreateInfo fence_create_info = {};
	
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext = nullptr;

		if(create_as_signaled)
		{
			// Create this fence in a signaled state!
			fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}

		std::shared_ptr<VkFence> new_fence = std::make_shared<VkFence>();

		VkResult result = vkCreateFence(device, &fence_create_info, nullptr, &(*new_fence));
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return std::nullopt;
		}

		// Insert the new fence into the fence map.
		add_entry(fence_name, new_fence);

		return new_fence;
	}

			
	std::optional<std::shared_ptr<VkFence>> VulkanFenceManager::get_fence(const std::string& fence_name)
	{
		assert(fence_manager_initialised);
		assert(fence_name.length()>0);

		if(!does_key_exist(fence_name))
		{
			spdlog::error("Vulkan fence '{}' does not exists!", fence_name);
			return std::nullopt;
		}

		return get_entry(fence_name);
	}
			
			
	void VulkanFenceManager::shutdown_fences()
	{
		assert(device);
		assert(fence_manager_initialised);
		
        // Use lock guard to ensure thread safety during write operations!
        std::lock_guard<std::mutex> lock(fence_manager_mutex);

		// Create an iterator for the unordered map.
		std::vector<std::shared_ptr<VkFence>> all_fences = get_all_values();
 
		// Iterate over the unordered map.
		for(auto& fence : all_fences)
		{
			// Destroy the fences.
			vkDestroyFence(device, *fence, nullptr);
		}

		delete_all_entries();
	}


};
};
