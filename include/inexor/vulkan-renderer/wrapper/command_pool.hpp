#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/queue_type.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkCommandPool.
class CommandPool {
    std::string m_name;
    const Device &m_device;
    const QueueType m_queue_type;
    const std::uint32_t m_queue_family_index;
    VkCommandPool m_cmd_pool{VK_NULL_HANDLE};

    /// The command buffers which can be requested by the current thread
    /// Each command buffer is created with ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT``
    std::vector<std::unique_ptr<CommandBuffer>> m_cmd_bufs;

public:
    /// Default constructor
    /// @param device The device wrapper instance
    /// @param queue_type The queue type
    /// @param name The internal debug marker name which will be assigned to this command pool
    CommandPool(Device &device, QueueType queue_type, std::string name);

    /// THIS WILL BE DELETED AS PART OF THE REFACTORING BUT NEEDS TO STAY IN THIS COMMIT TO KEEP COMMITS SMALL
    /// @note It is important that the queue family index is specified in the abstraction above this command pool
    /// wrapper. We can't choose one queue family index automatically inside of this wrapper which fits every purpose,
    /// because some wrappers require a queue family index which supports graphics bit, other require transfer bit.
    /// @param device The const reference to the device RAII wrapper class
    /// @param queue_family_index The queue family index which is used by this command pool
    /// @param name The internal debug marker name which will be assigned to this command pool
    CommandPool(const Device &device, std::uint32_t queue_family_index, std::string name);

    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) noexcept;

    ~CommandPool();

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = delete;

    [[nodiscard]] VkCommandPool get() const {
        return m_cmd_pool;
    }

    [[nodiscard]] const VkCommandPool *ptr() const {
        return &m_cmd_pool;
    }

    /// Request a command buffer
    /// @param name The internal debug name which will be assigned to this command buffer (must not be empty)
    /// @return A command buffer handle instance which allows access to the requested command buffer
    [[nodiscard]] const CommandBuffer &request_command_buffer(const std::string &name);
};

} // namespace inexor::vulkan_renderer::wrapper
