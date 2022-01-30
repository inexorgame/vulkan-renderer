#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

template <typename BufferDataType>
class UniformBuffer : public GPUMemoryBuffer {
public:
    UniformBuffer(const Device &device, const std::string &name)
        : GPUMemoryBuffer(device, name, sizeof(BufferDataType), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU) {}

    UniformBuffer(const Device &device, const BufferDataType &data, const std::string &name)
        : UniformBuffer(device, name) {
        update(data);
    }

    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer(UniformBuffer &&other) noexcept : GPUMemoryBuffer(std::move(other)) {}

    ~UniformBuffer() override = default;

    UniformBuffer &operator=(const UniformBuffer &) = delete;
    UniformBuffer &operator=(UniformBuffer &&) noexcept = default;

    void update(const BufferDataType *data) {
        assert(data);
        assert(m_allocation_info.pMappedData);
        std::memcpy(m_allocation_info.pMappedData, data, sizeof(BufferDataType));
    }

    // Sometimes we only need to update a part of the structure!
    template <typename T>
    void update(const T *data) {
        static_assert(sizeof(T) < sizeof(BufferDataType));
        assert(data);
        assert(m_allocation_info.pMappedData);
        std::memcpy(m_allocation_info.pMappedData, data, sizeof(T));
    }
};

} // namespace inexor::vulkan_renderer::wrapper
