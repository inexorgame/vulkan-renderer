#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const wrapper::Device &device, std::string name, const BufferType type,
               std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_type(type), m_on_update(std::move(on_update)) {
    // Uniform buffer can be updated by std::memcpy, other types of memory require staging buffer updates
    m_requires_staging_buffer_update = (type != BufferType::UNIFORM_BUFFER);
}

Buffer::Buffer(Buffer &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_type = other.m_type;
    m_on_update = std::move(other.m_on_update);
    m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
    m_alloc = std::exchange(other.m_alloc, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
    m_buf_usage = other.m_buf_usage;
    m_mem_usage = other.m_mem_usage;
}

Buffer::~Buffer() {
    destroy_buffer();
}

void Buffer::create_buffer(const VkDeviceSize buffer_size, const VkBufferUsageFlags buffer_usage,
                           const VmaMemoryUsage memory_usage) {
    // We don't support VK_SHARING_MODE_CONCURRENT for now
    const auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = buffer_size,
        .usage = buffer_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });
    // It is recommended to create the buffer as mapped and to keep it persistently mapped
    const VmaAllocationCreateInfo alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = memory_usage,
    };
    if (const auto result =
            vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &m_buffer, &m_alloc, &m_alloc_info)) {
        throw VulkanException("Error: vmaCreateBuffer failed for buffer " + m_name + " !", result);
    }

    // We are basically storing things duplicately here, but whatever
    m_buf_usage = buffer_usage;
    m_mem_usage = memory_usage;

    // Set the buffer's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());

    // Set the buffer's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_buffer, m_name);
}

void Buffer::destroy_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_alloc);
    // TODO: Should we do this?
    m_buffer = VK_NULL_HANDLE;
    m_alloc = VK_NULL_HANDLE;
}

void Buffer::recreate_buffer(const VkDeviceSize new_buffer_size) {
    destroy_buffer();
    // We are basically storing things duplicately here, but whatever
    create_buffer(new_buffer_size, m_buf_usage, m_mem_usage);
}

} // namespace inexor::vulkan_renderer::render_graph
