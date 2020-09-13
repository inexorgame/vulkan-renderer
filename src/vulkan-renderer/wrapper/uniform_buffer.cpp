#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <cassert>
#include <cstring>

namespace inexor::vulkan_renderer::wrapper {

UniformBuffer::UniformBuffer(const Device &device, const std::string &name, const VkDeviceSize &buffer_size)
    : GPUMemoryBuffer(device, name, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU) {}

UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept
    : GPUMemoryBuffer(std::move(other)), m_descriptor_buffer_info(std::move(other.m_descriptor_buffer_info)),
      m_descriptor_set(std::move(other.m_descriptor_set)) {}

void UniformBuffer::update(void *data, const std::size_t size) {
    assert(m_allocation_info.pMappedData);
    std::memcpy(m_allocation_info.pMappedData, data, size);
}

}; // namespace inexor::vulkan_renderer::wrapper
