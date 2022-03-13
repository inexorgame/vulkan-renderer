#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper {

CommandPool::CommandPool(const Device &device, const std::uint32_t queue_family_index, std::string name)
    : m_device(device), m_name(std::move(name)) {
    assert(device.device());

    auto command_pool_ci = make_info<VkCommandPoolCreateInfo>();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = queue_family_index;
    device.create_command_pool(command_pool_ci, &m_command_pool, m_name);
}

CommandPool::CommandPool(CommandPool &&other) noexcept : m_device(other.m_device) {
    m_command_pool = std::exchange(other.m_command_pool, nullptr);
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_device.device(), m_command_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
