#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class OnceCommandBuffer
/// @brief RAII wrapper class for VkCommandBuffers which will be used only once.
/// These types of command buffers are often used for copy operations. We're only going to use the command buffer once
/// and wait with returning from the function until the copy operation has finished executing. It's good practice to
/// tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
class OnceCommandBuffer {
    const Device &m_device;
    // We must store the VkQueue separately since we don't know from
    // the context of the use of this OnceCommandBuffer which queue to use!
    const VkQueue m_queue;
    CommandPool m_command_pool;
    std::unique_ptr<CommandBuffer> m_command_buffer;

    bool m_command_buffer_created{false};
    bool m_recording_started{false};

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param queue [in] The Vulkan queue to use.
    /// @param queue_family_index [in] The Vulkan queue family index to use.
    /// @warn We can't determine the queue and queue family index to use automatically using the device wrapper
    /// reference because we might choose a queue which is unsuitable for the requested purpose!
    /// This is the reason we must specify the queue and queue family index in the constructor.
    OnceCommandBuffer(const Device &device, const VkQueue queue, const std::uint32_t queue_family_index);
    OnceCommandBuffer(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer(OnceCommandBuffer &&) noexcept;
    ~OnceCommandBuffer() = default;

    OnceCommandBuffer &operator=(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer &operator=(OnceCommandBuffer &&) = default;

    /// @brief Creates the command buffer.
    /// @note We are not merging this into the constructor because we need to be able to call this function separately.
    void create_command_buffer();

    /// @brief Calls vkBeginCommandBuffer.
    void start_recording();

    /// @brief Calls vkEndCommandBuffer and vkFreeCommandBuffers.
    void end_recording_and_submit();

    [[nodiscard]] VkCommandBuffer command_buffer() const {
        return m_command_buffer->get();
    }
};

} // namespace inexor::vulkan_renderer::wrapper
