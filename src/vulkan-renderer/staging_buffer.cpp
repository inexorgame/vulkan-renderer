#include "inexor/vulkan-renderer/staging_buffer.hpp"

namespace inexor::vulkan_renderer {

StagingBuffer::StagingBuffer(StagingBuffer &&other) noexcept
    : GPUMemoryBuffer(std::move(other)), transfer_queue(std::exchange(other.transfer_queue, nullptr)),
      command_buffer(std::exchange(other.command_buffer, nullptr)) {}

StagingBuffer::StagingBuffer(const VkDevice &device, const VkCommandPool &command_pool, const VkCommandBuffer &command_buffer,
                             const VmaAllocator &vma_allocator, std::string &name, const VkDeviceSize &buffer_size, void *data, const std::size_t data_size)
    : GPUMemoryBuffer(device, vma_allocator, name, buffer_size, data, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      transfer_queue(transfer_queue), command_buffer(command_buffer) {}

void StagingBuffer::upload_data_to_gpu(const GPUMemoryBuffer &target_buffer, const VkQueue &data_transfer_queue) {
    VkBufferCopy vertex_buffer_copy = {};

    vertex_buffer_copy.srcOffset = 0;
    vertex_buffer_copy.dstOffset = 0;
    vertex_buffer_copy.size = allocation_info.size;

    VkCommandBufferBeginInfo buffer_copy_begin_info = {};

    buffer_copy_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_copy_begin_info.pNext = nullptr;
    buffer_copy_begin_info.pInheritanceInfo = nullptr;

    // We're only going to use the command buffer once and wait with returning from the function until the copy operation has finished executing.
    // It's good practice to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    buffer_copy_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    spdlog::debug("Beginning command buffer recording for copy of staging buffer for vertices.");

    if (vkBeginCommandBuffer(command_buffer, &buffer_copy_begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkBeginCommandBuffer failed for staging buffer of vertices of " + name + " !");
    }

    spdlog::debug("Specifying vertex buffer copy operation in command buffer.");

    vkCmdCopyBuffer(command_buffer, buffer, target_buffer.get_buffer(), 1, &vertex_buffer_copy);

    spdlog::debug("Ending command buffer recording for staging buffer copy.");

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkEndCommandBuffer failed for mesh buffer " + name + " !");
    }

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    if (vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueSubmit failed for mesh buffer " + name + " !");
    }

    if (vkQueueWaitIdle(data_transfer_queue) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueWaitIdle failed for mesh buffer " + name + " !");
    }

    spdlog::debug("Finished uploading mesh data to graphics card memory.");

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
}

StagingBuffer::~StagingBuffer() {
    // The staging buffer will be destroyed automatically when GPUMemoryBuffer's destructor is called.
}

} // namespace inexor::vulkan_renderer
