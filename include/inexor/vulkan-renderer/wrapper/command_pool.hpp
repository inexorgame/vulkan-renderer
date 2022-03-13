#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkCommandPool.
class CommandPool {
    std::string m_name;
    const Device &m_device;
    VkCommandPool m_command_pool{VK_NULL_HANDLE};

public:
    /// @brief Default constructor.
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
        return m_command_pool;
    }

    [[nodiscard]] const VkCommandPool *ptr() const {
        return &m_command_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
