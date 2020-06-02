#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A OnceCommandBuffer is a command buffer which is being used only once.
class OnceCommandBuffer {
private:
    wrapper::CommandPool command_pool;
    std::unique_ptr<wrapper::CommandBuffer> command_buffer;
    VkQueue data_transfer_queue;
    VkDevice device;

    bool command_buffer_created;
    bool recording_started;

public:
    // Delete the copy constructor so once command buffers are move-only objects.
    OnceCommandBuffer(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer(OnceCommandBuffer &&other) noexcept;

    // Delete the copy assignment operator so shaders are move-only objects.
    OnceCommandBuffer &operator=(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer &operator=(OnceCommandBuffer &&) noexcept = default;

    /// @brief Creates a new commandbuffer which is being called only once.
    /// @param device [in] The Vulkan device.
    /// @param data_transfer_queue [in] The data transfer queue.
    OnceCommandBuffer(const VkDevice device, const VkQueue data_transfer_queue,
                      const std::uint32_t data_transfer_queue_family_index);

    ~OnceCommandBuffer();

    void create_command_buffer();

    void start_recording();

    void end_recording_and_submit_command();

    [[nodiscard]] VkCommandBuffer get_command_buffer() const {
        return command_buffer->get();
    }
};

} // namespace inexor::vulkan_renderer::wrapper
