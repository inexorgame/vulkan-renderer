#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <mutex>
#include <vector>
#include <optional>
#include <unordered_map>

#include <Vulkan/vulkan.h>


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
			// TODO: Is accessing this a bottleneck? If so, we could call get_semaphore and setup before drawing.

			// @note VkSemaphores are actually nondispatchable handles. This means they can't be copied!
			std::unordered_map<std::string, VkSemaphore> semaphores;
			

		public:
			
			VulkanSynchronisationManager();
			
			~VulkanSynchronisationManager();


		protected:

		
			/// @brief Checks if a semaphore with this name already exists.
			/// @param semaphore_name The name of the semaphore.
			/// @return True if a Vulkan semaphore with this name already exists, false otherwise.
			bool does_semaphore_exist(const std::string& semaphore_name) const;


			/// @brief Creates a new Vulkan semaphore.
			/// @param semaphore_name The unique name of the semaphore.
			const std::optional<VkSemaphore> create_semaphore(const VkDevice& vulkan_device, const std::string& semaphore_name);


			/// @brief Gets a certain semaphore by name.
			/// @param semaphore_name The name of the semaphore.
			/// @return The acquired semaphore (if existent), std::nullopt otherwise.
			const std::optional<VkSemaphore> get_semaphore(const std::string& semaphore_name) const;


			/// @brief Destroys all existing semaphores.
			void shutdown_semaphores(const VkDevice& vulkan_device);


			// TODO: Implement Vulkan fences as well.

	};

};
};
