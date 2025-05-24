#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

BufferResource::BufferResource(const Device &device, std::string name, BufferType type)
    : ResourceBase(device, std::move(name)), m_buffer_type(type) {}

BufferResource::~BufferResource() {
    destroy();
}

void BufferResource::create(const CommandBuffer &cmd_buf) {
    if (m_src_data_size == 0) {
        spdlog::warn("Warning: Can't create buffer of size 0!");
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

    using wrapper::VulkanException;

    // The memory for the buffer we would like to create can end up in mappable memory, which means we can simply use
    // std::memcpy to copy the source date into it, or it ends up in non-mappable memory, which means we will need to
    // use a staging buffer and a transfer operation (a copy command) to upload the data to gpu memory. Which memory is
    // chosen by Vulkan Memory Allocator depends on the available memory and current memory usage.
    if (const auto result =
            vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &m_buffer, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed!", result, m_name);
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
        // This maps memory (if not mapped already), calls std::memcpy, and calls vmaFlushAllocation.
        vmaCopyMemoryToAllocation(m_device.allocator(), m_src_data, m_alloc, 0, m_src_data_size);

        // The allocation ended up in mappable memory and it is already mapped
        // This means we can simply use std::memcpy to copy the data from the source into it
        std::memcpy(m_alloc_info.pMappedData, m_src_data, m_src_data_size);

        // After copying the data, we need to flush caches
        // NOTE: vmaFlushAllocation checks internally if the memory is host coherent, in which case it don't flush
        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed for buffer!", result, m_name);
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
            throw VulkanException("Error: vmaCreateBuffer failed!", result, m_name);
        }

        const std::string staging_buf_name = "staging:" + m_name;
        // Set the staging buffer's internal debug name in Vulkan Memory Allocator (VMA)
        vmaSetAllocationName(m_device.allocator(), m_staging_buffer_alloc, staging_buf_name.c_str());
        // Set the staging buffer's internal debug name through Vulkan debug utils
        m_device.set_debug_name(m_staging_buffer, staging_buf_name);

        // Copy the memory into the staging buffer
        std::memcpy(m_staging_buffer_alloc_info.pMappedData, m_src_data, m_src_data_size);

        // NOTE: vmaFlushAllocation checks internally if the memory is host coherent, in which case it don't flush
        if (const auto result = vmaFlushAllocation(m_device.allocator(), m_staging_buffer_alloc, 0, VK_WHOLE_SIZE);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaFlushAllocation failed!", result, m_name);
        };

        cmd_buf.insert_debug_label("[Buffer::staging-update|" + m_name + "]",
                                   wrapper::get_debug_label_color(wrapper::DebugLabelColor::ORANGE));

        // TODO: Abstract like cmd_buf.pipeline_barrier(BUFFER_MEM_BARRIER_BEFORE_COPY, m_staging_buffer)
        //                     cmd_buf.pipeline_barrier(BUFFER_MEM_BARRIER_AFTER_COPY, m_staging_buffer)
        cmd_buf.pipeline_buffer_memory_barrier_before_copy_buffer(m_staging_buffer)
            .copy_buffer(m_staging_buffer, m_buffer, m_src_data_size)
            .pipeline_buffer_memory_barrier_after_copy_buffer(m_buffer);
    }

    // This is only required for uniform buffers really
    if (m_buffer_type == BufferType::UNIFORM_BUFFER) {
        m_descriptor_buffer_info = {
            .buffer = m_buffer,
            .offset = 0,
            .range = m_alloc_info.size,
        };
    }

    // The update is finished
    m_src_data = nullptr;
    m_src_data_size = 0;

    // NOTE: The staging buffer needs to stay valid until command buffer finished executing!
    // It will be destroyed either in the destructor or the next time the create method is called.
}

void BufferResource::destroy() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_alloc);
    m_buffer = VK_NULL_HANDLE;
    m_alloc = VK_NULL_HANDLE;
}

} // namespace inexor::vulkan_renderer::render_graph
