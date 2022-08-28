#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <thread>

namespace inexor::vulkan_renderer::wrapper {

CommandPool::CommandPool(Device &device, QueueType queue_type, std::string name)
    : m_device(device), m_queue_type(queue_type), m_name(std::move(name)),
      m_queue_family_index((queue_type == QueueType::GRAPHICS) ? m_device.graphics_queue_family_index()
                                                               : m_device.transfer_queue_family_index()) {
    auto cmd_pool_ci = make_info<VkCommandPoolCreateInfo>();
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_ci.queueFamilyIndex = device.graphics_queue_family_index();

    // Get the thread id as string for naming the command pool and the command buffers
    const std::size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    spdlog::trace("Creating command pool for thread {}", thread_id);

    if (const auto result = vkCreateCommandPool(m_device.device(), &cmd_pool_ci, nullptr, &m_cmd_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateCommandPool failed for command pool " + m_name + "!", result);
    }
}

/// THIS WILL BE DELETED AS PART OF THE REFACTORING BUT NEEDS TO STAY IN THIS COMMIT TO KEEP COMMITS SMALL
CommandPool::CommandPool(const Device &device, const std::uint32_t queue_family_index, std::string name)
    : m_device(device), m_name(std::move(name)), m_queue_type(QueueType::GRAPHICS),
      m_queue_family_index(m_device.graphics_queue_family_index()) {
    assert(device.device());

    auto command_pool_ci = make_info<VkCommandPoolCreateInfo>();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = queue_family_index;
    device.create_command_pool(command_pool_ci, &m_cmd_pool, m_name);
}

CommandPool::CommandPool(CommandPool &&other) noexcept
    : m_device(other.m_device), m_queue_type(other.m_queue_type), m_queue_family_index(other.m_queue_family_index) {
    m_cmd_pool = std::exchange(other.m_cmd_pool, nullptr);
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_device.device(), m_cmd_pool, nullptr);
}

const CommandBuffer &CommandPool::request_command_buffer(const std::string &name) {
    // Try to find a command buffer which is currently not used
    for (const auto &cmd_buf : m_cmd_bufs) {
        if (cmd_buf->fence_status() == VK_SUCCESS) {
            cmd_buf->reset_fence();
            return *cmd_buf;
        }
    }

    // We need to create a new command buffer because no free one was found
    m_cmd_bufs.emplace_back(std::make_unique<CommandBuffer>(m_device, m_cmd_pool, m_queue_type, "command buffer"));
    return *m_cmd_bufs.back();
}

} // namespace inexor::vulkan_renderer::wrapper
