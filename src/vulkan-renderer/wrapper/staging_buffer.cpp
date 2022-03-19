#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

StagingBuffer::StagingBuffer(const Device &device, const std::string &name, const VkDeviceSize buffer_size,
                             const void *data, const std::size_t data_size)
    : GPUMemoryBuffer(device, name, buffer_size, data, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VMA_MEMORY_USAGE_CPU_ONLY),
      m_device(device) {}

StagingBuffer::StagingBuffer(const Device &device, VkDeviceSize buffer_size, const void *data, const std::string &name)
    : StagingBuffer(device, name, buffer_size, data, buffer_size) {}

StagingBuffer::StagingBuffer(StagingBuffer &&other) noexcept
    : GPUMemoryBuffer(std::move(other)), m_device(other.m_device) {}

void StagingBuffer::upload_data_to_gpu(const GPUMemoryBuffer &target_buffer) {
    OnceCommandBuffer upload_command(m_device, [&](const CommandBuffer &cmd_buf) {
        cmd_buf.copy_buffer(m_buffer, target_buffer.buffer(), m_buffer_size);
    });
}

} // namespace inexor::vulkan_renderer::wrapper
