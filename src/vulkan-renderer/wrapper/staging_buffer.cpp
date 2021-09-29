#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

StagingBuffer::StagingBuffer(const Device &device, const std::string &name, const VkDeviceSize buffer_size, void *data,
                             const std::size_t data_size)
    : GPUMemoryBuffer(device, name, buffer_size, data, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VMA_MEMORY_USAGE_CPU_ONLY),
      m_command_buffer_for_copying(device, device.transfer_queue(), device.transfer_queue_family_index()),
      m_device(device) {}

StagingBuffer::StagingBuffer(StagingBuffer &&other) noexcept
    : m_command_buffer_for_copying(std::move(other.m_command_buffer_for_copying)), GPUMemoryBuffer(std::move(other)),
      m_device(other.m_device) {}

void StagingBuffer::upload_data_to_gpu(const GPUMemoryBuffer &tarbuffer) {
    spdlog::trace("Beginning command buffer recording for copy of staging buffer for vertices");

    m_command_buffer_for_copying.create_command_buffer();
    m_command_buffer_for_copying.start_recording();

    spdlog::trace("Specifying vertex buffer copy operation in command buffer");

    VkBufferCopy vertex_buffer_copy{};
    vertex_buffer_copy.srcOffset = 0;
    vertex_buffer_copy.dstOffset = 0;
    vertex_buffer_copy.size = m_buffer_size;

    vkCmdCopyBuffer(m_command_buffer_for_copying.command_buffer(), m_buffer, tarbuffer.buffer(), 1,
                    &vertex_buffer_copy);

    spdlog::trace("Finished uploading mesh data to graphics card memory");

    m_command_buffer_for_copying.end_recording_and_submit_command();

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
}

} // namespace inexor::vulkan_renderer::wrapper
