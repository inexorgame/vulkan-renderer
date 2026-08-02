#pragma once

#include <volk.h>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::commands {

/// RAII wrapper class for VkCommandPool
class CommandPool {
    std::string m_name;
    const core::Device &m_device;
    VkCommandPool m_cmd_pool{VK_NULL_HANDLE};
    VkQueueFlagBits m_queue_type;
    /// The command buffers which can be requested by the current thread
    std::vector<std::unique_ptr<CommandBuffer>> m_cmd_bufs;
    /// The secondary command buffers which can be requested by the current thread
    std::vector<CommandBuffer> m_secondary_cmd_bufs;
    /// Ring cursor used when all command buffers are currently in-flight.
    std::size_t m_next_reuse_index{0};
    /// Ring cursor used when all secondary command buffers are currently in-flight.
    std::size_t m_next_secondary_reuse_index{0};

    static constexpr std::size_t MAX_IN_FLIGHT_SUBMISSIONS = 3;

    /// Get the internal debug name of the command pool
    /// @note We use the queue type to generate a human-readable name for the command pool, but we dont want to use
    /// representation wrapper for this.
    std::string get_pool_name(const VkQueueFlagBits queue_type) const;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param queue_type The queue type
    /// @param queue_family_index The queue family index
    /// @param name The internal debug marker name which will be assigned to this command pool
    CommandPool(const core::Device &device, VkQueueFlagBits queue_type, std::uint32_t queue_family_index,
                std::string name);

    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) noexcept;

    ~CommandPool();

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = delete;

    /// Request a command buffer
    /// @param name The internal debug name which will be assigned to this command buffer (must not be empty)
    /// @return A command buffer handle instance which allows access to the requested command buffer
    [[nodiscard]] const CommandBuffer &request_command_buffer(const std::string &name);

    /// Request a secondary command buffer
    /// @param name The internal debug name which will be assigned to this command buffer (must not be empty)
    /// @return A command buffer handle instance which allows access to the requested command buffer
    [[nodiscard]] const CommandBuffer &request_secondary_command_buffer(const std::string &name);

    /// Wait until all submitted command buffers in this pool have finished execution.
    void wait_for_all_submissions() const;
};

} // namespace inexor::vulkan_renderer::wrapper::commands
