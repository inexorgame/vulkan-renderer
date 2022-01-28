#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for GPU Memory buffers.
/// Uniform buffers or vertex/index buffers use this as a base class.
/// @note The core of Inexor's memory management is Vulkan Memory Allocator library (VMA).
class GPUMemoryBuffer {
protected:
    std::string m_name;
    const Device &m_device;
    VkBuffer m_buffer{VK_NULL_HANDLE};
    VkDeviceSize m_buffer_size{0};
    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};
    VmaAllocationCreateInfo m_allocation_ci{};

public:
    VkDescriptorBufferInfo descriptor_buffer_info;

    /// @brief Construct the GPU memory buffer without specifying the actual data to fill in, only the memory size.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the GPU memory buffer.
    /// @param buffer_size The size of the memory buffer in bytes.
    /// @param buffer_usage The buffer usage flags.
    /// @param memory_usage The VMA memory usage flags which specify the required memory allocation.
    GPUMemoryBuffer(const Device &device, const std::string &name, const VkDeviceSize &size,
                    const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage);

    /// @brief Construct the GPU memory buffer and specifies the actual data to fill it in.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the GPU memory buffer.
    /// @param buffer_size The size of the memory buffer in bytes.
    /// @param data A pointer to the data to fill the GPU memory buffer with.
    /// @param data_size The size of the memory to copy from data pointer.
    /// @param buffer_usage The buffer usage flags.
    /// @param memory_usage The VMA memory usage flags which specify the required memory allocation.
    GPUMemoryBuffer(const Device &device, const std::string &name, const VkDeviceSize &buffer_size, const void *data,
                    std::size_t data_size, const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage);

    GPUMemoryBuffer(const GPUMemoryBuffer &) = delete;
    GPUMemoryBuffer(GPUMemoryBuffer &&) noexcept;

    virtual ~GPUMemoryBuffer();

    GPUMemoryBuffer &operator=(const GPUMemoryBuffer &) = delete;
    GPUMemoryBuffer &operator=(GPUMemoryBuffer &&) = delete;

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] VkBuffer buffer() const {
        return m_buffer;
    }

    [[nodiscard]] VmaAllocation allocation() const {
        return m_allocation;
    }

    [[nodiscard]] VmaAllocationInfo allocation_info() const {
        return m_allocation_info;
    }

    [[nodiscard]] VmaAllocationCreateInfo allocation_create_info() const {
        return m_allocation_ci;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
