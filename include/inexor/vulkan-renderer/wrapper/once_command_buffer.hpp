#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <functional>
#include <memory>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// @brief RAII wrapper class for VkCommandBuffers which will be used only once.
/// These types of command buffers are often used for copy operations. We're only going to use the command buffer once
/// and wait with returning from the function until the copy operation has finished executing. It's good practice to
/// tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
class OnceCommandBuffer : public CommandBuffer {
private:
    // TODO: Expose the command pool as argument in the constructor?
    // TODO: What's good practice when it comes to command pool creation? group by task? like graphics, copy operations?
    CommandPool m_command_pool;

public:
    OnceCommandBuffer(const Device &device, VkQueue queue, std::uint32_t queue_family_index,
                      std::function<void(const CommandBuffer &cmd_buf)> command_lambda,
                      std::string name = "once command");

    OnceCommandBuffer(const Device &device, std::function<void(const CommandBuffer &cmd_buf)> command_lambda,
                      std::string name = "once command");

    OnceCommandBuffer(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer(OnceCommandBuffer &&) noexcept;

    ~OnceCommandBuffer() = default;

    OnceCommandBuffer &operator=(const OnceCommandBuffer &) = delete;
    OnceCommandBuffer &operator=(OnceCommandBuffer &&) noexcept = default;

    // TODO: Add a method for the reuse of a OnceCommandBuffer instance (maybe we need to execute multiple single
    // commands after each other separately?)
};

} // namespace inexor::vulkan_renderer::wrapper
