#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <cassert>
#include <cstring>

namespace inexor::vulkan_renderer::wrapper {

UniformBuffer::UniformBuffer(const Device &device, const std::string &name, const VkDeviceSize &buffer_size)
    : GPUMemoryBuffer(device, name, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU) {}

UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept : GPUMemoryBuffer(std::move(other)) {}

void UniformBuffer::update(void *data, const std::size_t size) {
    std::memcpy(m_allocation_info.pMappedData, data, size);
}

}; // namespace inexor::vulkan_renderer::wrapper
