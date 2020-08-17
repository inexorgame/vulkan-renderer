#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper {

CommandPool::CommandPool(const VkDevice device, const std::uint32_t queue_family_index) : m_device(device) {
    assert(device);

    auto command_pool_ci = make_info<VkCommandPoolCreateInfo>();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = queue_family_index;

    if (vkCreateCommandPool(device, &command_pool_ci, nullptr, &m_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateCommandPool failed!");
    }

    // TODO: Assign an internal name to this command pool using Vulkan debug markers.

    spdlog::debug("Created command pool successfully.");
}

CommandPool::CommandPool(CommandPool &&other) noexcept
    : m_device(other.m_device), m_command_pool(std::exchange(other.m_command_pool, nullptr)) {}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
