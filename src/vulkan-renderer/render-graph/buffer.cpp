#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const Device &device, std::string buffer_name, BufferType buffer_type, std::function<void()> on_update)
    : m_device(device), m_name(std::move(buffer_name)), m_on_update(std::move(on_update)), m_buffer_type(buffer_type) {
    if (m_name.empty()) {
        throw std::invalid_argument("[Buffer::Buffer] Error: Parameter 'name' is empty!");
    }
}

Buffer::Buffer(Buffer &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_buffer_type = other.m_buffer_type;
    m_on_update = std::move(other.m_on_update);
    m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
    m_alloc = std::exchange(other.m_alloc, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
    m_src_data = std::exchange(other.m_src_data, m_src_data);
    m_src_data_size = other.m_src_data_size;
    m_staging_buffer = std::exchange(other.m_staging_buffer, VK_NULL_HANDLE);
    m_staging_buffer_alloc = std::exchange(other.m_staging_buffer_alloc, VK_NULL_HANDLE);
    m_staging_buffer_alloc_info = std::move(other.m_staging_buffer_alloc_info);
    m_update_requested = other.m_update_requested;
}

Buffer::~Buffer() {
    destroy();
}

void Buffer::create(const CommandBuffer &cmd_buf) {
    if (m_src_data_size == 0) {
        spdlog::warn("[Buffer::create_buffer] Warning: Can't create buffer of size 0!");
        return;
    }

    // This helps us to find the correct VkBufferUsageFlags depending on the BufferType
    const std::unordered_map<BufferType, VkBufferUsageFlags> BUFFER_USAGE{
        {BufferType::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        {BufferType::VERTEX_BUFFER, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
        {BufferType::INDEX_BUFFER, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
    };
    const auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = m_src_data_size,
        .usage = BUFFER_USAGE.at(m_buffer_type),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });
    // This helps us to find the correct VmaMemoryUsage depending on the BufferType
    const std::unordered_map<BufferType, VmaMemoryUsage> MEMORY_USAGE{
        {BufferType::UNIFORM_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_HOST},
        {BufferType::VERTEX_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
        {BufferType::INDEX_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
    };
    const VmaAllocationCreateInfo alloc_ci{
        .flags = (m_buffer_type == BufferType::UNIFORM_BUFFER)
                     ? static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                             VMA_ALLOCATION_CREATE_MAPPED_BIT)
                     : 0,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    // The memory for the buffer we would like to create can end up in mappable memory, which means we can simply use
    // std::memcpy to copy the source date into it, or it ends up in non-mappable memory, which means we will need to
    // use a staging buffer and a transfer operation (a copy command) to upload the data to gpu memory. Which memory is
    // chosen by Vulkan Memory Allocator depends on the available memory and current memory usage.
    if (const auto result =
            vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &m_buffer, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed for buffer " + m_name + " !", result);
    }

    // Set the buffer's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the buffer's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_buffer, m_name);

    // Query memory property flags
    VkMemoryPropertyFlags mem_prop_flags{};
    vmaGetAllocationMemoryProperties(m_device.allocator(), m_alloc, &mem_prop_flags);

    // Check if the allocation made by VMA ended up in mappable memory
    if (mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        // The allocation ended up in mappable memory and it is already mapped
        // This means we can simply use std::memcpy to copy the data from the source into it
        std::memcpy(m_alloc_info.pMappedData, m_src_data, m_src_data_size);

        // After copying the data, we need to flush caches
        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed for buffer " + m_name + " !", result);
        }
    } else {
        // Make sure to destroy the previous staging buffer
        if (m_staging_buffer != VK_NULL_HANDLE) {
            destroy_staging_buffer();
        }
        // The allocation ended up in non-mappable memory and we need a staging buffer and a copy command to upload data
        const auto staging_buf_ci = wrapper::make_info<VkBufferCreateInfo>({
            // The size of the staging buffer must be the size of the actual buffer
            .size = m_src_data_size,
            // This is the buffer usage bit for staging buffers
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        });
        const VmaAllocationCreateInfo staging_buf_alloc_ci{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        // Create the staging buffer which is used for the transfer of data into the actual buffer
        if (const auto result =
                vmaCreateBuffer(m_device.allocator(), &staging_buf_ci, &staging_buf_alloc_ci, &m_staging_buffer,
                                &m_staging_buffer_alloc, &m_staging_buffer_alloc_info);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaCreateBuffer failed for staging buffer " + m_name + " !", result);
        }

        const std::string staging_buf_name = "staging:" + m_name;
        // Set the staging buffer's internal debug name in Vulkan Memory Allocator (VMA)
        vmaSetAllocationName(m_device.allocator(), m_staging_buffer_alloc, staging_buf_name.c_str());
        // Set the staging buffer's internal debug name through Vulkan debug utils
        m_device.set_debug_name(m_staging_buffer, staging_buf_name);

        // Copy the memory into the staging buffer
        std::memcpy(m_staging_buffer_alloc_info.pMappedData, m_src_data, m_src_data_size);

        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_staging_buffer_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed for staging buffer " + m_name + " !", result);
        };

        cmd_buf.pipeline_buffer_memory_barrier_before_copy_buffer(m_staging_buffer)
            .copy_buffer(m_staging_buffer, m_buffer, m_src_data_size)
            .pipeline_buffer_memory_barrier_after_copy_buffer(m_buffer);
    }
    // Update the descriptor buffer info
    m_descriptor_buffer_info = VkDescriptorBufferInfo{
        .buffer = m_buffer,
        .offset = 0,
        .range = m_alloc_info.size,
    };
    // The update is finished
    m_update_requested = false;
}

void Buffer::destroy() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_alloc);
    m_buffer = VK_NULL_HANDLE;
    m_alloc = VK_NULL_HANDLE;
    destroy_staging_buffer();
}

void Buffer::destroy_staging_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_staging_buffer_alloc);
    m_staging_buffer = VK_NULL_HANDLE;
    m_staging_buffer_alloc = VK_NULL_HANDLE;
}

void Buffer::request_update(void *src_data, const std::size_t src_data_size) {
    if (src_data == nullptr || src_data_size == 0) {
        return;
    }
    m_src_data = src_data;
    m_src_data_size = src_data_size;
    m_update_requested = true;
}

} // namespace inexor::vulkan_renderer::render_graph
