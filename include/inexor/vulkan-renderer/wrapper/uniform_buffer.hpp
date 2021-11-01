#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <cstring>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for uniform buffers.
class UniformBuffer : public GPUMemoryBuffer {
public:
    // TODO: Change order of parameters so name is last!

    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the uniform buffer.
    /// @param size The size of the uniform buffer.
    /// @todo Add overloaded constructor which directly accepts the uniform buffer data.
    UniformBuffer(const Device &device, const std::string &name, const VkDeviceSize &size);

    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer(UniformBuffer &&) noexcept;

    ~UniformBuffer() override = default;

    UniformBuffer &operator=(const UniformBuffer &) = delete;
    UniformBuffer &operator=(UniformBuffer &&) = delete;

    /// @brief Update uniform buffer data.
    /// @param data A pointer to the uniform buffer data.
    template <typename T>
    void update(const T *data) {
        assert(data);
        // Note that the size of type T must always be greater than 0 according to C++ standard.
        std::memcpy(m_allocation_info.pMappedData, data, sizeof(T));
    }

    VkDescriptorBufferInfo descriptor;
    VkDescriptorSet descriptor_set;
};

} // namespace inexor::vulkan_renderer::wrapper
