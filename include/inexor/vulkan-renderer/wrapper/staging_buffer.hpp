#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {

/// @brief In general, it is inefficient to use normal memory mapping to a vertex buffer.
/// It is highly advised to use a staging buffer. Once the staging buffer is filled with data,
/// a queue command can be executed to use a transfer queue to upload the data to the GPU memory.
class StagingBuffer : public GPUMemoryBuffer {
private:
    VkQueue m_data_transfer_queue;
    OnceCommandBuffer m_command_buffer_for_copying;

public:
    /// @brief Creates a new staging buffer.
    /// @param device [in] The Vulkan device from which the buffer will be created.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param data [in] The address of the data which will be copied.
    /// @param size [in] The size of the buffer in bytes.
    /// @note Staging buffers always have VK_BUFFER_USAGE_TRANSFER_SRC_BIT as VkBufferUsageFlags.
    /// @note Staging buffers always have VMA_MEMORY_USAGE_CPU_ONLY as VmaMemoryUsage.
    StagingBuffer(wrapper::Device &device, const VmaAllocator vma_allocator, const VkQueue data_transfer_queue,
                  const std::uint32_t data_transfer_queueu_family_index, const std::string &name,
                  const VkDeviceSize buffer_size, void *data, const std::size_t data_size);
    StagingBuffer(const StagingBuffer &) = delete;
    StagingBuffer(StagingBuffer &&) noexcept;
    ~StagingBuffer() = default;

    StagingBuffer &operator=(const StagingBuffer &) = delete;
    StagingBuffer &operator=(StagingBuffer &&) = default;

    ///
    void upload_data_to_gpu(const GPUMemoryBuffer &tarbuffer);
};

} // namespace inexor::vulkan_renderer::wrapper
