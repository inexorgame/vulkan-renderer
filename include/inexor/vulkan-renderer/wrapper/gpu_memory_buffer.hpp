#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class GPUMemoryBuffer {
protected:
    std::string m_name;

    VkDevice m_device;
    VmaAllocator m_vma_allocator;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceSize m_buffer_size = 0;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo m_allocation_info{};
    VmaAllocationCreateInfo m_allocation_ci{};

public:
    /// Delete the copy constructor so gpu memory buffers are move-only objects.
    GPUMemoryBuffer(const GPUMemoryBuffer &) = delete;
    GPUMemoryBuffer(GPUMemoryBuffer &&buffer) noexcept;

    /// Delete the copy assignment operator so gpu memory buffers are move-only objects.
    GPUMemoryBuffer &operator=(const GPUMemoryBuffer &) = delete;
    GPUMemoryBuffer &operator=(GPUMemoryBuffer &&) noexcept = default;

    /// @brief Creates a new GPU memory buffer.
    /// @param device [in] The Vulkan device from which the buffer will be created.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param size [in] The size of the buffer in bytes.
    /// @param buffer_usage [in] The Vulkan buffer usage flags.
    /// @param memory_usage [in] The Vulkan Memory Allocator library's memory usage flags.
    GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, const std::string &name,
                    const VkDeviceSize &size, const VkBufferUsageFlags &buffer_usage,
                    const VmaMemoryUsage &memory_usage);

    /// @brief Creates a new GPU memory buffer.
    /// @param device [in] The Vulkan device from which the buffer will be created.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param buffer_size [in] The size of the buffer in bytes.
    /// @param data [in] The address of the data which will be copied.
    /// @param data_size [in] The size of the data which will be copied.
    /// @param buffer_usage [in] The Vulkan buffer usage flags.
    /// @param memory_usage [in] The Vulkan Memory Allocator library's memory usage flags.
    GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, const std::string &name,
                    const VkDeviceSize &buffer_size, void *data, const std::size_t data_size,
                    const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage);

    virtual ~GPUMemoryBuffer();

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] const VkBuffer buffer() const {
        return m_buffer;
    }

    [[nodiscard]] const VmaAllocation allocation() const {
        return m_allocation;
    }

    [[nodiscard]] const VmaAllocationInfo allocation_info() const {
        return m_allocation_info;
    }

    [[nodiscard]] const VmaAllocationCreateInfo allocation_create_info() const {
        return m_allocation_ci;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
