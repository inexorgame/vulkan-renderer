#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include "inexor/vulkan-renderer/exceptions/vk_exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper {

CommandPool::CommandPool(const Device &device, const std::uint32_t queue_family_index) : m_device(device) {
    assert(device.device());

    auto command_pool_ci = make_info<VkCommandPoolCreateInfo>();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = queue_family_index;

    if (const auto result = vkCreateCommandPool(m_device.device(), &command_pool_ci, nullptr, &m_command_pool);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkCreateCommandPool failed!", result);
    }

    // TODO: Assign an internal name to this command pool using Vulkan debug markers.

    spdlog::debug("Created command pool successfully.");
}

CommandPool::CommandPool(CommandPool &&other) noexcept
    : m_device(other.m_device), m_command_pool(std::exchange(other.m_command_pool, nullptr)) {}

CommandPool::~CommandPool() {
    if (m_command_pool != nullptr) {
        vkDestroyCommandPool(m_device.device(), m_command_pool, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
