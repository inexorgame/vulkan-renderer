#include "inexor/vulkan-renderer/once_command_buffer.hpp"

namespace inexor::vulkan_renderer {

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : device(std::exchange(other.device, nullptr)), command_pool(std::exchange(other.command_pool, nullptr)),
      command_buffer(std::exchange(other.command_buffer, nullptr)), data_transfer_queue(std::exchange(other.data_transfer_queue, nullptr)),
      data_transfer_queue_family_index(other.data_transfer_queue_family_index), recording_started(other.recording_started) {}

OnceCommandBuffer::OnceCommandBuffer(const VkDevice device, const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index)
    : device(device), data_transfer_queue(data_transfer_queue), data_transfer_queue_family_index(data_transfer_queue_family_index) {

    spdlog::debug("Creating command pool for rendering.");

    VkCommandPoolCreateInfo command_pool_create_info = {};

    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = data_transfer_queue_family_index;

    if (vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateCommandPool failed for once command buffer!");
    }

    // TODO: Rename all "allocation_info" variables in the engine to "alloc_info".
    VkCommandBufferAllocateInfo command_buffer_alloc_info = {};

    command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_alloc_info.pNext = nullptr;
    command_buffer_alloc_info.commandBufferCount = 1;
    command_buffer_alloc_info.commandPool = command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(device, &command_buffer_alloc_info, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateCommandBuffers failed for once command buffer!");
    }

    // TODO: Set object name using Vulkan debug markers.
}

OnceCommandBuffer::~OnceCommandBuffer() {
    assert(device);
    assert(command_pool);
    vkDestroyCommandPool(device, command_pool, nullptr);
}

void OnceCommandBuffer::start_recording() {
    assert(device);
    assert(command_pool);
    assert(command_buffer);
    assert(data_transfer_queue);
    assert(!recording_started);

    spdlog::debug("Starting recording of once command buffer.");

    VkCommandBufferBeginInfo command_buffer_begin_info = {};

    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;

    // We're only going to use the command buffer once and wait with returning from the function until the copy operation has finished executing.
    // It's good practice to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkBeginCommandBuffer failed for once command buffer!");
    }

    recording_started = true;

    // TODO: Set object name using Vulkan debug markers.
}

void OnceCommandBuffer::end_recording_and_submit_command() {
    assert(device);
    assert(command_pool);
    assert(command_buffer);
    assert(data_transfer_queue);
    assert(recording_started);

    spdlog::debug("Ending recording of once command buffer.");

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: VkEndCommandBuffer failed for once command buffer!");
    }

    spdlog::debug("Command buffer recording ended successfully.");

    spdlog::debug("Starting to submit command.");

    VkSubmitInfo submit_info = {};

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    if (vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueSubmit failed for once command buffer!");
    }

    // TODO: Refactor! Introduce proper synchronisation using VkFence!
    if (vkQueueWaitIdle(data_transfer_queue) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueWaitIdle failed for once command buffer!");
    }

    spdlog::debug("Destroying once command buffer.");

    // Because we destroy the command buffer after submission, we have to allocate it every time.
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);

    recording_started = false;
}

} // namespace inexor::vulkan_renderer
