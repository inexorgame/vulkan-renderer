#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

GPUMemoryBuffer::GPUMemoryBuffer(const Device &device, const std::string &name, const VkDeviceSize &size,
                                 const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage)
    : m_device(device), m_name(name), m_buffer_size(size) {
    assert(device.device());
    assert(device.allocator());
    assert(!name.empty());

    spdlog::debug("Creating GPU memory buffer of size {} for '{}'.", size, name);

    auto buffer_ci = make_info<VkBufferCreateInfo>();
    buffer_ci.size = size;
    buffer_ci.usage = buffer_usage;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    m_allocation_ci.usage = memory_usage;

#if VMA_RECORDING_ENABLED
    m_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    m_allocation_ci.pUserData = m_name.data();
#else
    m_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    // TODO: Should we create this buffer as mapped?
    // TODO: Is it good to have memory mapped all the time?
    // TODO: When should memory be mapped / unmapped?

    if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &m_allocation_ci, &m_buffer,
                                            &m_allocation, &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: GPU memory buffer allocation for " + name + " failed!", result);
    }

    // Assign an internal debug marker name to this buffer.
    m_device.set_debug_marker_name(m_buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
}

GPUMemoryBuffer::GPUMemoryBuffer(const Device &device, const std::string &name, const VkDeviceSize &buffer_size,
                                 void *data, const std::size_t data_size, const VkBufferUsageFlags &buffer_usage,
                                 const VmaMemoryUsage &memory_usage)
    : GPUMemoryBuffer(device, name, buffer_size, buffer_usage, memory_usage) {
    assert(device.device());
    assert(device.allocator());
    assert(!name.empty());
    assert(buffer_size > 0);
    assert(data_size > 0);
    assert(data);

    // Copy the memory into the buffer!
    std::memcpy(m_allocation_info.pMappedData, data, data_size);
}

GPUMemoryBuffer::GPUMemoryBuffer(GPUMemoryBuffer &&other) noexcept
    : m_name(std::move(other.m_name)), m_device(other.m_device), m_buffer(std::exchange(other.m_buffer, nullptr)),
      m_allocation(std::exchange(other.m_allocation, nullptr)), m_allocation_info(std::move(other.m_allocation_info)),
      m_allocation_ci(std::move(other.m_allocation_ci)) {}

GPUMemoryBuffer::~GPUMemoryBuffer() {
    if (m_buffer != nullptr) {
        vmaDestroyBuffer(m_device.allocator(), m_buffer, m_allocation);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
