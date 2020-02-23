#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <assert.h>

#include <glm/glm.hpp>


namespace inexor {
namespace vulkan_renderer {


	// Predefined color markers.
	// These colors will be visible in RenderDoc.
	#define INEXOR_DEBUG_MARKER_BLUE   glm::vec4(0.0f, 148/255, 1.0f, 1.0f)
	#define INEXOR_DEBUG_MARKER_RED    glm::vec4(1.0f, 0.0f, 21/255, 1.0f)
	#define INEXOR_DEBUG_MARKER_YELLOW glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
	#define INEXOR_DEBUG_MARKER_PURPLE glm::vec4(1.0f, 0.0f, 180/255, 1.0f)
	#define INEXOR_DEBUG_MARKER_GREEN  glm::vec4(40/255, 210/255, 0.0f, 1.0f)
	#define INEXOR_DEBUG_MARKER_ORANGE glm::vec4(1.0f, 100/255, 0.0f, 1.0f)


	/// @class VulkanDebugMarkerManager
	/// @brief A manager class for Vulkan debug markers.
	/// Debug markers are very useful because they allow the single steps of the renderer
	class VulkanDebugMarkerManager
	{
		private:
		
			bool active = false;

			bool extension_present = false;

			PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
			
			PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
			
			PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = VK_NULL_HANDLE;
			
			PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = VK_NULL_HANDLE;
			
			PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = VK_NULL_HANDLE;


		public:

			/// @brief Initialises Vulkan debug marker manager.
			/// @param device The Vulkan device.
			/// @param graphics_card The graphics card.
			VulkanDebugMarkerManager(const VkDevice& device, const VkPhysicalDevice& graphics_card);
			
			
			~VulkanDebugMarkerManager();

			
			// Sets the debug name of an object
			// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
			// along with the object type
			void set_object_name(const VkDevice& device, const uint64_t& object, const VkDebugReportObjectTypeEXT& object_type, const char *name);


			// Set the tag for an object
			void set_object_tag(const VkDevice& device, const uint64_t& object, const VkDebugReportObjectTypeEXT& object_type, const uint64_t& name, const std::size_t& tag_size, const void* tag);


			// Start a new debug marker region
			void bind_region(const VkCommandBuffer& cmdbuffer, const std::string& debug_marker_name, const glm::vec4& color);


			// Insert a new debug marker into the command buffer
			void insert(const VkCommandBuffer& command_buffer, const std::string& debug_marker_name, glm::vec4 debug_marker_color);


			// End the current debug marker region
			void end_region(const VkCommandBuffer& command_buffer);


	};

};
};
