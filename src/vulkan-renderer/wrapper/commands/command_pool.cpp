#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <thread>

namespace inexor::vulkan_renderer::wrapper::commands {

using tools::VulkanException;

CommandPool::CommandPool(const Device &device, std::string name) : m_device(device), m_name(std::move(name)) {
    const auto cmd_pool_ci = make_info<VkCommandPoolCreateInfo>({
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = device.graphics_queue_family_index(),
    });

    // Get the thread id as string for naming the command pool and the command buffers
    const std::size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    spdlog::trace("Creating command pool for thread {}", thread_id);

    if (const auto result = vkCreateCommandPool(m_device.device(), &cmd_pool_ci, nullptr, &m_cmd_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateCommandPool failed for command pool " + m_name + "!", result);
    }
}

CommandPool::CommandPool(CommandPool &&other) noexcept : m_device(other.m_device) {
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
            cmd_buf->set_debug_name(name);
            cmd_buf->begin_command_buffer();
            return *cmd_buf;
        }
    }

    // We need to create a new command buffer because no free one was found
    // Note that there is currently no method for shrinking m_cmd_bufs, but this should not be a problem
    m_cmd_bufs.emplace_back(std::make_unique<CommandBuffer>(m_device, m_cmd_pool, "command buffer"));

    spdlog::trace("Creating new command buffer #{}", m_cmd_bufs.size());

    m_cmd_bufs.back()->begin_command_buffer();
    return *m_cmd_bufs.back();
}

} // namespace inexor::vulkan_renderer::wrapper::commands
