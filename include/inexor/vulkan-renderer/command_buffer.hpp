#pragma once

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <cassert>
#include <string>

namespace inexor::vulkan_renderer {
class CommandBuffer {
protected:
    std::string name = "";
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

public:
    /// Delete the copy constructor so command buffers are move-only types.
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&other) noexcept;

    /// Delete the copy assignment operator so command buffers are move-only objects.
    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) noexcept = default;

    /// @brief Creates a new command buffer.
    CommandBuffer(const VkDevice &device, const std::string &name, const VkCommandPool &command_pool);

    const VkCommandBuffer get_command_buffer() const {
        return command_buffer;
    }

    ~CommandBuffer();
};
} // namespace inexor::vulkan_renderer
