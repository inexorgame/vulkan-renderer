#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class CommandPool {
    const Device &m_device;
    VkCommandPool m_command_pool;

public:
    /// @brief Creates a Vulkan command pool.
    /// @param device [in] The Vulkan device.
    /// @param queue_family_index [in] The queue family index for the command pool.
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
