#include "inexor/vulkan-renderer/command_pool.hpp"

namespace inexor::vulkan_renderer {

CommandPool::CommandPool(CommandPool &&other) noexcept
    : name(std::move(name)), device(std::exchange(other.device, nullptr)), command_pool(std::exchange(other.command_pool, nullptr)),
      data_transfer_queue_family_index(other.data_transfer_queue_family_index) {}

CommandPool::CommandPool(const VkDevice &device, const std::string &name, const std::uint32_t data_transfer_queue_family_index)
    : device(device), name(name), data_transfer_queue_family_index(data_transfer_queue_family_index)

{
    VkCommandPoolCreateInfo command_pool_create_info = {};

    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = data_transfer_queue_family_index;

    spdlog::debug("Creating command pool {}.", name);

    // Create a second command pool for all commands that are going to be executed in the data transfer queue.
    if (vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateCommandPool failed for command pool " + name + " !");
    }
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(device, command_pool, nullptr);
}
} // namespace inexor::vulkan_renderer
