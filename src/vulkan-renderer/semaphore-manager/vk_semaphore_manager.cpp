#include "vk_semaphore_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"


namespace inexor {
namespace vulkan_renderer {

				
	VulkanSemaphoreManager::VulkanSemaphoreManager()
	{
	}


	VulkanSemaphoreManager::~VulkanSemaphoreManager()
	{
	}

	
	VkResult VulkanSemaphoreManager::initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager)
	{
		assert(device);
		assert(debug_marker_manager);

		spdlog::debug("Initialising semaphore manager.");

		// Use lock guard to ensure thread safety.
		std::lock_guard<std::mutex> lock(semaphore_manager_mutex);

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;

		semaphore_manager_initialised = true;

		return VK_SUCCESS;
	}


	bool VulkanSemaphoreManager::does_semaphore_exist(const std::string& semaphore_name)
	{
		assert(semaphore_manager_initialised);
		assert(semaphore_name.length()>0);

		// Call template base class method.
		return does_key_exist(semaphore_name);
	}


	std::optional<std::shared_ptr<VkSemaphore>> VulkanSemaphoreManager::create_semaphore(const std::string& semaphore_name)
	{
		assert(device);
		assert(semaphore_manager_initialised);
		assert(semaphore_name.length()>0);
		
		// First check if a Vulkan semaphore with this name already exists!
		if(does_semaphore_exist(semaphore_name))
		{
			spdlog::error("Semaphore '{}' does already exist!", semaphore_name);
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
		std::shared_ptr<VkSemaphore> new_semaphore = std::make_shared<VkSemaphore>();

		// TODO: Does that work?
		VkResult result = vkCreateSemaphore(device, &semaphore_create_info, nullptr, &(*new_semaphore));
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return std::nullopt;
		}
		
		// Insert the semaphore into the semaphore map.
		add_entry(semaphore_name, new_semaphore);

		return new_semaphore;
	}


	std::optional<std::shared_ptr<VkSemaphore>> VulkanSemaphoreManager::get_semaphore(const std::string& semaphore_name)
	{
		assert(semaphore_manager_initialised);
		assert(semaphore_name.length()>0);

		if(!does_key_exist(semaphore_name))
		{
			spdlog::error("Semaphore '{}' does not exist!", semaphore_name);
			return std::nullopt;
		}

		// Call template base class method.
		return get_entry(semaphore_name);
	}


	void VulkanSemaphoreManager::shutdown_semaphores()
	{
		assert(device);
		assert(semaphore_manager_initialised);
		
		spdlog::debug("Destroying semaphores.");

		// TODO: Get as unordered map!
		auto all_semaphores = get_all_values();

		// Use lock guard to ensure thread safety.
		std::lock_guard<std::mutex> lock(semaphore_manager_mutex);

		for(auto& semaphore : all_semaphores)
		{
			vkDestroySemaphore(device, *semaphore, nullptr);
		}

		// Call template base class method.
		delete_all_entries();
	}


};
};
