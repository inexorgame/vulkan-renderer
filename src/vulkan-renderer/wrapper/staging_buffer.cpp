#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

StagingBuffer::StagingBuffer(StagingBuffer &&other) noexcept
    : data_transfer_queue(std::move(other.data_transfer_queue)),
      command_buffer_for_copying(std::move(other.command_buffer_for_copying)), GPUMemoryBuffer(std::move(other)) {}

StagingBuffer::StagingBuffer(const VkDevice device, const VmaAllocator vma_allocator, const VkQueue data_transfer_queue,
                             const std::uint32_t data_transfer_queueu_family_index, const std::string &name,
                             const VkDeviceSize buffer_size, void *data, const std::size_t data_size)
    : GPUMemoryBuffer(device, vma_allocator, name, buffer_size, data, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VMA_MEMORY_USAGE_CPU_ONLY),
      data_transfer_queue(data_transfer_queue),
      command_buffer_for_copying(device, data_transfer_queue, data_transfer_queueu_family_index) {}

void StagingBuffer::upload_data_to_gpu(const GPUMemoryBuffer &target_buffer) {
    spdlog::debug("Beginning command buffer recording for copy of staging buffer for vertices.");

    command_buffer_for_copying.create_command_buffer();

    command_buffer_for_copying.start_recording();

    spdlog::debug("Specifying vertex buffer copy operation in command buffer.");

    VkBufferCopy vertex_buffer_copy{};
    vertex_buffer_copy.srcOffset = 0;
    vertex_buffer_copy.dstOffset = 0;
    vertex_buffer_copy.size = buffer_size;

    vkCmdCopyBuffer(command_buffer_for_copying.get_command_buffer(), buffer, target_buffer.get_buffer(), 1,
                    &vertex_buffer_copy);

    spdlog::debug("Finished uploading mesh data to graphics card memory.");

    command_buffer_for_copying.end_recording_and_submit_command();

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
}

} // namespace inexor::vulkan_renderer::wrapper
