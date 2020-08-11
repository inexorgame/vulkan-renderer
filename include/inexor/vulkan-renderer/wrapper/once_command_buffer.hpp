#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A OnceCommandBuffer is a command buffer which is being used only once.
class OnceCommandBuffer {
private:
    wrapper::CommandPool m_command_pool;
    std::unique_ptr<wrapper::CommandBuffer> m_command_buffer;
    VkQueue m_data_transfer_queue;
    wrapper::Device &m_device;

    bool m_command_buffer_created;
    bool m_recording_started;

public:
    /// @brief Creates a new commandbuffer which is being called only once.
    /// @param device [in] The Vulkan device.
    /// @param data_transfer_queue [in] The data transfer queue.
    OnceCommandBuffer(wrapper::Device &device, const VkQueue data_transfer_queue,
                      const std::uint32_t data_transfer_queue_family_index);
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
