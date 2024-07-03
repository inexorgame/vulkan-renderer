#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const Device &device, std::string buffer_name, BufferType buffer_type, std::function<void()> on_update)
    : m_device(device), m_name(std::move(buffer_name)), m_on_update(std::move(on_update)), m_buffer_type(buffer_type) {}

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
    m_staging_alloc = std::exchange(other.m_staging_alloc, VK_NULL_HANDLE);
    m_staging_alloc_info = std::move(other.m_staging_alloc_info);
}

Buffer::~Buffer() {
    destroy_buffer();
}

void Buffer::create_buffer(const CommandBuffer &cmd_buf) {
    const std::unordered_map<BufferType, VkBufferUsageFlags> BUFFER_USAGE{
        {BufferType::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        {BufferType::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
        {BufferType::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
    };
    const auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = m_src_data_size,
        .usage = BUFFER_USAGE.at(m_buffer_type),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO: Support VK_SHARING_MODE_CONCURRENT
    });
    const std::unordered_map<BufferType, VmaAllocationCreateFlags> BUFFER_FLAGS{
        {BufferType::UNIFORM_BUFFER, VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT},
        {BufferType::VERTEX_BUFFER,
         VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT},
        {BufferType::INDEX_BUFFER,
         VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT},
    };
    const std::unordered_map<BufferType, VmaMemoryUsage> MEMORY_USAGE{
        {BufferType::UNIFORM_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_HOST},
        {BufferType::VERTEX_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
        {BufferType::INDEX_BUFFER, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
    };
    const VmaAllocationCreateInfo alloc_ci{
        .flags = BUFFER_FLAGS.at(m_buffer_type),
        .usage = MEMORY_USAGE.at(m_buffer_type),
    };

    if (const auto result =
            vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &m_buffer, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed for buffer " + m_name + " !", result);
    }

    // Set the buffer's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the buffer's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_buffer, m_name);

    // Ask VMA which memory was chosen for the allocation
    VkMemoryPropertyFlags mem_prop_flags{};
    vmaGetAllocationMemoryProperties(m_device.allocator(), m_alloc, &mem_prop_flags);

    // Upload the data depending on whether we can use std::memcpy or require a staging buffer
    if (mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        // The allocation ended up in mappable memory and it is already mapped
        std::memcpy(m_alloc_info.pMappedData, m_src_data, m_src_data_size);
        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed for buffer " + m_name + " !", result);
        }
    } else {
        // The allocation ended up in non-mappable memory and we need a staging buffer to upload the data
        const auto staging_buf_ci = wrapper::make_info<VkBufferCreateInfo>({
            .size = m_src_data_size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        });
        const VmaAllocationCreateInfo staging_buf_alloc_ci{
            // TODO: Decide this depending on m_buffer_type!
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        // Create the staging buffer which is used for the transfer of data into the actual buffer
        if (const auto result = vmaCreateBuffer(m_device.allocator(), &staging_buf_ci, &staging_buf_alloc_ci,
                                                &m_staging_buffer, &m_staging_alloc, &m_staging_alloc_info);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaCreateBuffer failed for staging buffer " + m_name + " !", result);
        }

        // Set the staging buffer's internal debug name in Vulkan Memory Allocator (VMA)
        vmaSetAllocationName(m_device.allocator(), m_staging_alloc, m_name.c_str());
        // Set the staging buffer's internal debug name through Vulkan debug utils
        m_device.set_debug_name(m_staging_buffer, m_name);

        // Copy the memory into the staging buffer
        std::memcpy(m_staging_alloc_info.pMappedData, m_src_data, m_src_data_size);

        // TODO: Decide here if flush is required as well?
        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_staging_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed for staging buffer " + m_name + " !", result);
        };

        cmd_buf
            // TODO: Maybe even abstract this further and use only .pipeline_buffer_memory_barrier(/*arguments*/)?
            // Yeah we definitely need better abstraction for barriers...
            .pipeline_buffer_memory_barrier(VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                            {
                                                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                                                .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                                .buffer = m_staging_buffer,
                                                .offset = 0,
                                                .size = VK_WHOLE_SIZE,
                                            })
            // Further abstraction is required here as well...
            .copy_buffer(m_staging_buffer, m_buffer,
                         {
                             .srcOffset = 0,
                             .dstOffset = 0,
                             .size = m_src_data_size,
                         })
            .pipeline_buffer_memory_barrier(
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                {
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT, // TODO: Depending on buffer type!
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = m_staging_buffer,
                    .offset = 0,
                    .size = VK_WHOLE_SIZE,
                });
    }
}

void Buffer::destroy_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_alloc);
    vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_alloc);
}

} // namespace inexor::vulkan_renderer::render_graph
