#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {
class CommandPool {
private:
    VkDevice device;
    VkCommandPool command_pool;

public:
    /// Delete the copy constructor so Vulkan command pools are move-only objects.
    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&other) noexcept;

    /// Delete the copy assignment operator so Vulkan command pools are move-only objects.
    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) noexcept = default;

    /// @brief Creates a Vulkan command pool.
    /// @param device [in] The Vulkan device.
    /// @param queue_family_index [in] The queue family index for the command pool.
    CommandPool(const VkDevice device, const std::uint32_t queue_family_index);

    ~CommandPool();

    [[nodiscard]] VkCommandPool get() const {
        return command_pool;
    }

    [[nodiscard]] const VkCommandPool *get_ptr() const {
        return &command_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
