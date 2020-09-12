#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class CommandPool
/// @brief RAII wrapper class for VkCommandPool.
class CommandPool {
    const Device &m_device;
    VkCommandPool m_command_pool;

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param queue_family_index [in] The queue family index which is used by this command pool.
    /// @note It is important that the queue family index is specified in the abstraction
    /// above this command pool wrapper. We can't choose one queue family index automatically
    /// inside of this wrapper which fits every purpose!
    CommandPool(const Device &device, const std::uint32_t queue_family_index);
    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) noexcept;
    ~CommandPool();

    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = default;

    [[nodiscard]] VkCommandPool get() const {
        return m_command_pool;
    }

    [[nodiscard]] const VkCommandPool *ptr() const {
        return &m_command_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
