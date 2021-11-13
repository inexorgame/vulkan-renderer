#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

class Device;

template <typename BufferDataType>
class UniformBuffer : public GPUMemoryBuffer {
private:
    VkDescriptorBufferInfo m_descriptor{VK_NULL_HANDLE};

public:
    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};

    ///
    ///
    ///
    UniformBuffer(const Device &device, const std::string &name)
        : GPUMemoryBuffer(device, name, sizeof(BufferDataType), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU) {

        m_descriptor.buffer = m_buffer;
        m_descriptor.offset = 0;
        m_descriptor.range = sizeof(BufferDataType);
    }

    ///
    ///
    ///
    ///
    UniformBuffer(const Device &device, const BufferDataType &data, const std::string &name)
        : UniformBuffer(device, name) {
        update(data);
    }

    UniformBuffer(const UniformBuffer &) = delete;

    UniformBuffer(UniformBuffer &&other) noexcept : GPUMemoryBuffer(std::move(other)) {
        m_descriptor = std::exchange(other.m_descriptor, nullptr);
        m_descriptor_set = std::exchange(other.m_descriptor_set, nullptr);
    }

    ~UniformBuffer() override = default;

    UniformBuffer &operator=(const UniformBuffer &) = delete;
    UniformBuffer &operator=(UniformBuffer &&) = delete;

    void update(const BufferDataType *data) {
        assert(data);
        assert(m_allocation_info.pMappedData);
        std::memcpy(m_allocation_info.pMappedData, data, sizeof(BufferDataType));
    }

    [[nodiscard]] auto descriptor() const {
        return m_descriptor;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
