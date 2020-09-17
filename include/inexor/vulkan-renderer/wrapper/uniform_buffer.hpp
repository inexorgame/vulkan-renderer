#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class UniformBuffer : public GPUMemoryBuffer {
protected:
    VkDescriptorBufferInfo m_descriptor_buffer_info{};
    VkDescriptorSet m_descriptor_set{};

public:
    /// @brief Creates a new uniform buffer.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param size [in] The size of the buffer in bytes.
    /// @param buffer_usage [in] The Vulkan buffer usage flags.
    /// @param memory_usage [in] The Vulkan Memory Allocator library's memory usage flags.
    UniformBuffer(const Device &device, const std::string &name, const VkDeviceSize &size);

    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer(UniformBuffer &&) noexcept;

    ~UniformBuffer() = default;

    UniformBuffer &operator=(const UniformBuffer &) = delete;
    UniformBuffer &operator=(UniformBuffer &&) = default;

    /// @brief Updates the data of a uniform buffer.
    void update(void *data, const std::size_t size);
};

} // namespace inexor::vulkan_renderer::wrapper
