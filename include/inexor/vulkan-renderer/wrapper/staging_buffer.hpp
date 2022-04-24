#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// @brief RAII wrapper class for staging buffers.
/// A staging buffer is a buffer which is used for copying data.
/// Using a staging buffer is the most efficient way to copy memory from RAM to GPU.
class StagingBuffer final : public GPUMemoryBuffer {
    const Device &m_device;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the staging buffer.
    /// @param buffer_size The size of the memory buffer to copy.
    /// @param data A pointer to the memory buffer.
    /// @param data_size The size of the memory buffer to copy.
    StagingBuffer(const Device &device, const std::string &name, VkDeviceSize buffer_size, const void *data,
                  std::size_t data_size);

    /// Create a staging buffer which has buffer size equal to data size
    ///
    ///
    ///
    ///
    StagingBuffer(const Device &device, VkDeviceSize buffer_size, const void *data, const std::string &name);

    StagingBuffer(const StagingBuffer &) = delete;
    StagingBuffer(StagingBuffer &&) noexcept;
    ~StagingBuffer() override = default;

    StagingBuffer &operator=(const StagingBuffer &) = delete;
    StagingBuffer &operator=(StagingBuffer &&) noexcept = default;

    /// @brief Call vkCmdCopyBuffer inside of the once command buffer.
    /// @param tarbuffer The memory buffer to copy.
    void upload_data_to_gpu(const GPUMemoryBuffer &tarbuffer, std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper
