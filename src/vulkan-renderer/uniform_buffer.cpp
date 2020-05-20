#include "inexor/vulkan-renderer/uniform_buffer.hpp"

#include <cassert>
#include <cstring>

namespace inexor::vulkan_renderer {

// TODO: Fix move constructor!
UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept
    : GPUMemoryBuffer(std::move(other)), descriptor_buffer_info(std::move(other.descriptor_buffer_info)), descriptor_set(std::move(other.descriptor_set)) {}

UniformBuffer::UniformBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, const std::string &name, const VkDeviceSize &buffer_size)
    : GPUMemoryBuffer(device, vma_allocator, name, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU) {}

void UniformBuffer::update(void *data, const std::size_t size) {
    assert(allocation_info.pMappedData);

    std::memcpy(allocation_info.pMappedData, data, size);
}

}; // namespace inexor::vulkan_renderer
