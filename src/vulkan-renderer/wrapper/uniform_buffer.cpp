#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <cassert>
#include <cstring>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

UniformBuffer::UniformBuffer(const Device &device, const std::string &name, const VkDeviceSize &buffer_size)
    : GPUMemoryBuffer(device, name, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU) {

    // TODO: Is this technically correct to do here?
    descriptor.buffer = m_buffer;
    descriptor.offset = 0;
    descriptor.range = buffer_size;
}

UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept : GPUMemoryBuffer(std::move(other)) {}

} // namespace inexor::vulkan_renderer::wrapper
