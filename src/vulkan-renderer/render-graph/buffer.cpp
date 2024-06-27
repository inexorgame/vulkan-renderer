#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const Device &device, std::string name, std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_on_update(std::move(on_update)),
      m_buffer_type(BufferType::UNIFORM_BUFFER) {
    // TODO: Set buffer usage flags!
}

Buffer::Buffer(const Device &device,
               std::string name,
               std::vector<VkVertexInputAttributeDescription> vert_input_attr_descs,
               std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_on_update(std::move(on_update)),
      m_buffer_type(BufferType::VERTEX_BUFFER) {
    // TODO: Set buffer usage flags!
}

Buffer::Buffer(const Device &device,
               std::string name,
               VkIndexType index_type,
               std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_on_update(std::move(on_update)), m_index_type(index_type),
      m_buffer_type(BufferType::INDEX_BUFFER) {
    // TODO: Set buffer usage flags!
}

Buffer::Buffer(Buffer &&other) noexcept
    : m_device(other.m_device), m_buffer_usage(other.m_buffer_usage), m_mem_usage(other.m_mem_usage) {
    // TODO: Fix me!
    m_name = std::move(other.m_name);
    m_buffer_type = other.m_buffer_type;
    m_on_update = std::move(other.m_on_update);
    m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
    m_alloc = std::exchange(other.m_alloc, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
}

Buffer::~Buffer() {
    destroy_buffer();
}

void Buffer::create_buffer() {
    const auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = m_src_data_size,
        .usage = m_buffer_usage,
        // TODO: Support VK_SHARING_MODE_CONCURRENT
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });
    const VmaAllocationCreateInfo alloc_ci{
        // It is recommended to create the buffer as mapped and to keep it persistently mapped
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = m_mem_usage,
    };
    if (const auto result =
            vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &m_buffer, &m_alloc, &m_alloc_info)) {
        throw VulkanException("Error: vmaCreateBuffer failed for buffer " + m_name + " !", result);
    }
    // Set the buffer's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the buffer's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_buffer, m_name);
}

void Buffer::update_buffer() {
    // Before updating the buffer, we must check if the required new size is bigger than the existing buffer
    if (m_src_data_size > m_alloc_info.size) {
        // If a bigger buffer is required, we need to destroy and recreate the buffer
        destroy_buffer();
        // Note that m_src_data_size is already set, we don't have to pass any arguments
        create_buffer();
    }
    // Depending on the buffer type, different update mechanisms must be applied
    switch (m_buffer_type) {
    case BufferType::UNIFORM_BUFFER: {
        // Uniform buffers can be updated simply by using memcpy
        std::memcpy(m_alloc_info.pMappedData, m_src_data, m_src_data_size);
        break;
    }
    case BufferType::VERTEX_BUFFER:
    case BufferType::INDEX_BUFFER: {
        // Vertex and index buffers are updated through a buffer copy command using a staging buffer
        // This itself requires another instance of the Buffer wrapper class
        // Note that staging buffers are managed inside of device wrapper, not rendergraph

        // TODO: Batching of pipeline memory barriers for staging buffer through rendergraph?
        break;
    }
        /*
        case BufferType::STAGING_BUFFER: {
            // A staging buffer does not require updates, as it itself is used to update other buffers
            // Note that staging buffers are managed inside of device wrapper, not rendergraph, and we currently do not
            // re-use staging buffers
            break;
        }
        */
    }
}

void Buffer::destroy_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_alloc);
}

} // namespace inexor::vulkan_renderer::render_graph
