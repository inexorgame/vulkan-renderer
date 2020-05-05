#pragma once

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>

namespace inexor::vulkan_renderer {

/// @brief A OnceCommandBuffer is a command buffer which is being used only once.
class OnceCommandBuffer {
private:
    VkDevice device = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkQueue data_transfer_queue = VK_NULL_HANDLE;
    std::uint32_t data_transfer_queue_family_index = 0;

    bool recording_started = false;

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
    OnceCommandBuffer(const VkDevice device, const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index);

    ~OnceCommandBuffer();

    void start_recording();

    void end_recording_and_submit_command();

    const VkCommandBuffer get_command_buffer() const {
        return command_buffer;
    }
};

} // namespace inexor::vulkan_renderer
