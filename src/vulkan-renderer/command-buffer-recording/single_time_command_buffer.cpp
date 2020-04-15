#include "single_time_command_buffer.hpp"


namespace inexor {
namespace vulkan_renderer {


	VkResult SingleTimeCommandBufferRecorder::initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const VkQueue& data_transfer_queue)
	{
		assert(device);
		assert(debug_marker_manager);

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;
		this->data_transfer_queue = data_transfer_queue;

		command_buffer_recorder_initialised = true;

		return VK_SUCCESS;
	}


	VkResult SingleTimeCommandBufferRecorder::start_recording_of_single_time_command_buffer()
	{
		assert(device);
		assert(command_buffer_recorder_initialised);

		spdlog::debug("Starting recording command buffer for single time command.");

		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.pNext              = nullptr;
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		// We have to allocate the command buffer every time this function is
		// called because we call vkFreeCommandBuffers in end_single_time_commands.
		VkResult result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
		vulkan_error_check(result);

		VkCommandBufferBeginInfo command_buffer_begin_info = {};

		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext = nullptr;

		// We're only going to use the command buffer once and wait with returning from the function until
		// the copy operation has finished executing. It's good practice to tell the driver about our intent
		// using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Begin recording of the command buffer.
		result = vkBeginCommandBuffer(data_transfer_command_buffer, &command_buffer_begin_info);
		vulkan_error_check(result);

		// 
		debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_buffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Command buffer for ?.");

		return result;
	}


	VkResult SingleTimeCommandBufferRecorder::end_recording_of_single_time_command_buffer()
	{
		assert(device);
		assert(data_transfer_queue);
		assert(data_transfer_command_buffer);
		assert(command_buffer_recorder_initialised);

		spdlog::debug("Ending recording command buffer for single time command.");

		VkResult result = vkEndCommandBuffer(data_transfer_command_buffer);
		vulkan_error_check(result);

		VkSubmitInfo submit_info = {};

		submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &data_transfer_command_buffer;

		spdlog::debug("Submitting commands to GPU.");

		result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		vulkan_error_check(result);

		// TODO: Refactor and use a VkFence instead!
		result = vkQueueWaitIdle(data_transfer_queue);
		vulkan_error_check(result);
		
		spdlog::debug("Destroying command buffer.");

		// Because we destroy the command buffer after submission, we have to allocate it every time.
		vkFreeCommandBuffers(device, data_transfer_command_pool, 1, &data_transfer_command_buffer);

		return VK_SUCCESS;
	}


	void SingleTimeCommandBufferRecorder::destroy_command_pool()
	{
		assert(device);
		assert(data_transfer_command_pool);

		spdlog::debug("Destroying command pool.");

		vkDestroyCommandPool(device, data_transfer_command_pool, nullptr);
	}


};
};
