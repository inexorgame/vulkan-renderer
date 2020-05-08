﻿#pragma once

#include "vma/vk_mem_alloc.h"

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer {

class GPUMemoryBuffer {
protected:
    std::string name;

    VkDevice device;
    VmaAllocator vma_allocator;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info = {};
    VkBufferCreateInfo create_info = {};
    VmaAllocationCreateInfo allocation_create_info = {};

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
    GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, std::string &name, const VkDeviceSize &size,
                    const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage);

    /// @brief Creates a new GPU memory buffer.
    /// @param device [in] The Vulkan device from which the buffer will be created.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param buffer_size [in] The size of the buffer in bytes.
    /// @param data [in] The address of the data which will be copied.
    /// @param data_size [in] The size of the data which will be copied.
    /// @param buffer_usage [in] The Vulkan buffer usage flags.
    /// @param memory_usage [in] The Vulkan Memory Allocator library's memory usage flags.
    GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, std::string &name, const VkDeviceSize &buffer_size, void *data,
                    const std::size_t data_size, const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage);

    const std::string &get_name() const {
        return name;
    }
    const VkBuffer get_buffer() const {
        return buffer;
    }

    const VmaAllocation get_allocation() const {
        return allocation;
    }

    const VmaAllocationInfo get_allocation_info() const {
        return allocation_info;
    }

    const VkBufferCreateInfo get_buffer_create_info() const {
        return create_info;
    }

    const VmaAllocationCreateInfo get_allocation_create_info() const {
        return allocation_create_info;
    }

    ~GPUMemoryBuffer();
};

} // namespace inexor::vulkan_renderer
