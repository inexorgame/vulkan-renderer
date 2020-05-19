#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {
class CommandBuffer {
private:
    VkCommandBuffer command_buffer;

public:
    /// Delete the copy constructor so command buffers are move-only objects.
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&other) noexcept;

    /// Delete the copy assignment operator so command buffers are move-only objects.
    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&other) noexcept = default;

    CommandBuffer(const VkDevice device, const VkCommandPool command_pool);

    /// @note We don't need to destroy the command buffer because it will be destroyed along with it's associated command pool automatically.

    [[nodiscard]] VkCommandBuffer get() const {
        return command_buffer;
    }

    [[nodiscard]] const VkCommandBuffer *get_ptr() const {
        return &command_buffer;
    }
};
} // namespace inexor::vulkan_renderer::wrapper
