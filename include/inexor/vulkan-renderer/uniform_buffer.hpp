#pragma once

#include "gpu_memory_buffer.hpp"

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>

#include <shared_mutex>
#include <string>

namespace inexor::vulkan_renderer {

class UniformBuffer : public GPUMemoryBuffer {
protected:
    VkDescriptorBufferInfo descriptor_buffer_info = {};
    VkDescriptorSet descriptor_set = {};

public:
    // Delete the copy constructor so uniform buffers are move-only objects.
    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer(UniformBuffer &&buffer) noexcept;

    // Delete the copy assignment operator so uniform buffers are move-only objects.
    UniformBuffer &operator=(const UniformBuffer &) = delete;
    UniformBuffer &operator=(UniformBuffer &&) noexcept = default;

    /// @brief Creates a new uniform buffer.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param name [in] The internal name of the buffer.
    /// @param size [in] The size of the buffer in bytes.
    /// @param buffer_usage [in] The Vulkan buffer usage flags.
    /// @param memory_usage [in] The Vulkan Memory Allocator library's memory usage flags.
    UniformBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, std::string &name, const VkDeviceSize &size);

    /// @brief Updates the data of a uniform buffer.
    void update(void *data, const std::size_t size);

    ~UniformBuffer();
};

} // namespace inexor::vulkan_renderer
