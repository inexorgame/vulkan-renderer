#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class StagingBuffer
/// @brief RAII wrapper class for staging buffers.
/// A staging buffer is a buffer which is used for copying data.
/// Using a staging buffer is the most efficient way to copy memory from RAM to GPU.
class StagingBuffer : public GPUMemoryBuffer {
    const Device &m_device;
    OnceCommandBuffer m_command_buffer;

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param name [in] The internal debug marker name of the staging buffer.
    /// @param buffer_size [in] The size of the memory buffer to copy.
    /// @param data [in] A pointer to the memory buffer.
    /// @param data_size [in] The size of the memory buffer to copy.
    StagingBuffer(const Device &device, const std::string &name, const VkDeviceSize buffer_size, void *data,
                  const std::size_t data_size);

    StagingBuffer(const StagingBuffer &) = delete;
    StagingBuffer(StagingBuffer &&) noexcept;

    StagingBuffer &operator=(const StagingBuffer &) = delete;
    StagingBuffer &operator=(StagingBuffer &&) = default;

    /// @brief
    /// @param tarbuffer
    void upload_data_to_gpu(const GPUMemoryBuffer &tarbuffer);
};

} // namespace inexor::vulkan_renderer::wrapper
