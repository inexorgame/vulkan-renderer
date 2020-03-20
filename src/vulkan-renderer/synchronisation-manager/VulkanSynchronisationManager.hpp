#pragma once

#include "../error-handling/error-handling.hpp"

#include <mutex>
#include <vector>
#include <optional>
#include <unordered_map>

#include <assert.h>

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {

	
	/// @class VulkanSynchronisationManager
	/// @brief Creates and destroys Vulkan fences and semaphores.
	/// Those are essential for the synchronisation of multithreaded rendering and asynchronous code in general!
	/// @note Fences are mainly designed to synchronize your application itself with rendering operation,
	/// whereas semaphores are used to synchronize operations within or across command queues.
	class VulkanSynchronisationManager
	{
		private:

			// The stored semaphores.
			std::unordered_map<std::string, VkSemaphore> semaphores;
			
			// The stored fences.
			std::unordered_map<std::string, VkFence> fences;

			// The mutex of this class.
			std::mutex vulkan_synchronisation_manager_mutex;


		public:
			
			VulkanSynchronisationManager();
			
			~VulkanSynchronisationManager();


		protected:

		
			/// @brief Checks if a semaphore with this name already exists.
			/// @param semaphore_name The name of the semaphore.
			/// @return True if a Vulkan semaphore with this name already exists, false otherwise.
			bool does_semaphore_exist(const std::string& semaphore_name) const;


			/// @brief Creates a new Vulkan semaphore.
			/// @param vulkan_device The Vulkan device handle.
			/// @param semaphore_name The unique name of the semaphore.
			const std::optional<VkSemaphore> create_semaphore(const VkDevice& vulkan_device, const std::string& semaphore_name);


			/// @brief Gets a certain semaphore by name.
			/// @param semaphore_name The name of the semaphore.
			/// @return The acquired semaphore (if existent), std::nullopt otherwise.
			const std::optional<VkSemaphore> get_semaphore(const std::string& semaphore_name) const;


			/// @brief Destroys all existing semaphores.
			/// @param vulkan_device The Vulkan device handle.
			void shutdown_semaphores(const VkDevice& vulkan_device);

			
			/// @brief Checks if a fence with this name already exists.
			/// @param fence_name The name of the fence.
			/// @return True if a Vulkan fence with this name already exists, false otherwise.
			bool does_fence_exist(const std::string& fence_name) const;

			
			/// @brief Creates a new Vulkan fence.
			/// @param vulkan_device The Vulkan device handle.
			/// @param fence_name The unique name of the fence.
			const std::optional<VkFence> create_fence(const VkDevice& vulkan_device, const std::string& fence_name, bool create_as_signaled = true);

			
			/// @brief Gets a certain fence by name.
			/// @param fence_name The name of the fence.
			/// @return The acquired fence (if existent), std::nullopt otherwise.
			const std::optional<VkFence> get_fence(const std::string& fence_name) const;

			
			/// @brief Destroys all existing fences.
			/// @param vulkan_device The Vulkan device handle.
			void shutdown_fences(const VkDevice& vulkan_device);

	};

};
};
