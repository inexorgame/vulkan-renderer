#include "inexor/vulkan-renderer/wrapper/buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cstring>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Buffer::Buffer(const Device &device, const VkDeviceSize buffer_size, const VkBufferUsageFlags buffer_usage,
               const VmaMemoryUsage memory_usage, std::string name)
    : m_device(device), m_name(std::move(name)) {
    // Make sure to give every buffer an internal debug name that is not empty
    if (m_name.empty()) {
        throw std::invalid_argument("Error: Buffer name must not be empty!");
    }

    const auto buffer_ci = make_info<VkBufferCreateInfo>({
        .size = buffer_size,
        .usage = buffer_usage,
        // We don't support VK_SHARING_MODE_CONCURRENT for now
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });

    const VmaAllocationCreateInfo allocation_ci{
        // It is recommended to create the buffer as mapped and to keep it persistently mapped
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = memory_usage,
    };

    if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &allocation_ci, &m_buffer, &m_allocation,
                                            &m_allocation_info)) {
        throw VulkanException("Error: vmaCreateBuffer failed for buffer " + m_name + " !", result);
    }

    vmaSetAllocationName(m_device.allocator(), m_allocation, m_name.c_str());
}

Buffer::Buffer(const Device &device, const VkDeviceSize buffer_size, const void *buffer_data,
               const VkBufferUsageFlags buffer_usage, const VmaMemoryUsage memory_usage, std::string name)
    // Create the buffer by calling the default constructor
    : Buffer(device, buffer_size, buffer_usage, memory_usage, std::move(name)) {
    // Copy the buffer data into the buffer
    std::memcpy(m_allocation_info.pMappedData, buffer_data, buffer_size);
}

Buffer::~Buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_allocation);
}

Buffer::Buffer(Buffer &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
    m_allocation = std::exchange(other.m_allocation, VK_NULL_HANDLE);
    m_allocation_info = other.m_allocation_info;
}

} // namespace inexor::vulkan_renderer::wrapper
