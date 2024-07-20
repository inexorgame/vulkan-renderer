#pragma once

#include <spdlog/spdlog.h>
#include <volk.h>

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {

// Using declaration
using wrapper::Device;

/// RAII wrapper class for VkCommandPool
class CommandPool {
    std::string m_name;
    const Device &m_device;
    VkCommandPool m_cmd_pool{VK_NULL_HANDLE};
    VkQueueFlagBits m_queue_type;

    /// The command buffers which can be requested by the current thread
    mutable std::vector<std::unique_ptr<CommandBuffer>> m_cmd_bufs;

public:
    /// Default constructor
    /// @param device The device wrapper instance
    /// @param queue_type The queue type
    /// @param name The internal debug marker name which will be assigned to this command pool
    CommandPool(const Device &device, VkQueueFlagBits queue_type, std::string name);

    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) noexcept;
    ~CommandPool();

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = delete;

    /// Request a command buffer
    /// @param name The internal debug name which will be assigned to this command buffer (must not be empty)
    /// @return A command buffer handle instance which allows access to the requested command buffer
    [[nodiscard]] CommandBuffer &request_command_buffer(const std::string &name) const;
};

} // namespace inexor::vulkan_renderer::wrapper::commands
