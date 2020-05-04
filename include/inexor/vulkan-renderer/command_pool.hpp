#pragma once

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <string>

namespace inexor::vulkan_renderer {

class CommandPool {
private:
    std::string name = "";

    VkDevice device;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    std::uint32_t data_transfer_queue_family_index = 0;

public:
    // Delete the copy constructor so command pools are move-only objects.
    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&other) noexcept;

    // Delete the copy assignment operator so uniform buffers are move-only objects.
    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) noexcept = default;

    /// @brief Creates a new command pool.
    /// @param name [in] The internal name of the command pool.
    /// @param data_transfer_queue_family_index [in] The index of the queue family which will be used for data transfer.
    CommandPool(const VkDevice &device, const std::string &name, std::uint32_t data_transfer_queue_family_index);

    const VkCommandPool get_command_pool() const {
        return command_pool;
    }

    ~CommandPool();
};

} // namespace inexor::vulkan_renderer
