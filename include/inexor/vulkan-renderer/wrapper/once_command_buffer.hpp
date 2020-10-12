#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief A OnceCommandBuffer is a command buffer which is being used only once.
class OnceCommandBuffer {
    const Device &m_device;
    // We must store the VkQueue separately since we don't know from
    // the context of the use of this OnceCommandBuffer which queue to use!
    const VkQueue m_queue;
    CommandPool m_command_pool;
    std::unique_ptr<CommandBuffer> m_command_buffer;

    bool m_command_buffer_created;
    bool m_recording_started;

public:
    /// @brief Creates a new commandbuffer which is being called only once.
    /// @param device [in] A const reference to the Vulkan device.
    OnceCommandBuffer(const Device &device, const VkQueue queue, const std::uint32_t queue_family_index);

    OnceCommandBuffer(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer(OnceCommandBuffer &&) noexcept;

    ~OnceCommandBuffer();

    OnceCommandBuffer &operator=(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer &operator=(OnceCommandBuffer &&) = default;

    void create_command_buffer();

    void start_recording();

    void end_recording_and_submit_command();

    [[nodiscard]] VkCommandBuffer command_buffer() const {
        return m_command_buffer->get();
    }
};

} // namespace inexor::vulkan_renderer::wrapper
