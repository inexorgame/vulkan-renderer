#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

namespace inexor::vulkan_renderer::wrapper {

CommandPool::CommandPool(CommandPool &&other) noexcept : device(other.device), command_pool(std::exchange(other.command_pool, nullptr)) {}

CommandPool::CommandPool(const VkDevice device, const std::uint32_t queue_family_index) : device(device) {
    assert(device);

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = queue_family_index;

    if (vkCreateCommandPool(device, &create_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateCommandPool failed!");
    }

    // TODO: Assign an internal name to this command pool using Vulkan debug markers.

    spdlog::debug("Created command pool successfully.");
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(device, command_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
