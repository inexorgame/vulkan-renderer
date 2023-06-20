#pragma once

#include <vk_mem_alloc.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// A wrapper class for VkBuffer using Vulkan Memory Allocator (VMA)
class Buffer {
private:
    const Device &m_device;
    std::string m_name;

    VkBuffer m_buffer{VK_NULL_HANDLE};
    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};

public:
    /// Creates a VkBuffer using Vulkan Memory Allocator (VMA)
    /// @param device The device wrapper
    /// @param buffer_size The buffer size (must not be 0)
    /// @note If buffer_size is 0, vmaCreateBuffer will fail and an exception will be thrown
    /// @param buffer_usage The buffer usage flags
    /// @param memory usage The VMA memory usage flags
    /// @param name The internal debug name of the buffer (must not be empty)
    /// @exception std::invalid_argument The internal debug name of the buffer is empty
    /// @exception VulkanException vmaCreateBuffer call failed
    Buffer(const Device &device, VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
           std::string name);

    /// Creates a VkBuffer using Vulkan Memory Allocator (VMA) and copies memory into the buffer
    /// @param device The device wrapper
    /// @param buffer_size The buffer size (must not be 0)
    /// @note If buffer_size is 0, vmaCreateBuffer will fail and an exception will be thrown
    /// @param buffer_data A pointer to the buffer data (must not be nullptr)
    /// @param buffer_usage The buffer usage flags
    /// @param memory usage The VMA memory usage flags
    /// @param name The internal debug name of the buffer (must not be empty)
    /// @exception std::invalid_argument The internal debug name of the buffer is empty
    /// @exception VulkanException vmaCreateBuffer call failed
    Buffer(const Device &device, VkDeviceSize buffer_size, const void *buffer_data, VkBufferUsageFlags buffer_usage,
           VmaMemoryUsage memory_usage, std::string name);

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    virtual ~Buffer();

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    [[nodiscard]] VmaAllocationInfo allocation_info() const noexcept {
        return m_allocation_info;
    }

    [[nodiscard]] VkBuffer buffer() const noexcept {
        return m_buffer;
    }

    [[nodiscard]] auto memory() const noexcept {
        return m_allocation_info.pMappedData;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
