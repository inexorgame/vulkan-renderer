#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <thread>

namespace inexor::vulkan_renderer::wrapper::commands {

CommandPool::CommandPool(const Device &device, const VkQueueFlagBits queue_type, std::string name)
    : m_device(device), m_queue_type(queue_type), m_name(std::move(name)) {

    auto get_queue_family_index = [&]() {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT: {
            return m_device.transfer_queue_family_index();
        }
        case VK_QUEUE_COMPUTE_BIT: {
            return m_device.compute_queue_family_index();
        }
        default: {
            // VK_QUEUE_GRAPHICS_BIT and rest
            return m_device.graphics_queue_family_index();
        }
        }
    };

    const auto cmd_pool_ci = make_info<VkCommandPoolCreateInfo>({
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = get_queue_family_index(),
    });

    // Get the thread id as string for naming the command pool and the command buffers
    std::ostringstream thread_id;
    thread_id << std::this_thread::get_id();
    spdlog::trace("Creating {} command pool for thread ID {}", vk_tools::as_string(queue_type), thread_id.str());

    if (const auto result = vkCreateCommandPool(m_device.device(), &cmd_pool_ci, nullptr, &m_cmd_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateCommandPool failed for command pool " + m_name + "!", result);
    }
    m_device.set_debug_name(m_cmd_pool, m_name);
}

CommandPool::CommandPool(CommandPool &&other) noexcept : m_device(other.m_device) {
    m_cmd_pool = std::exchange(other.m_cmd_pool, nullptr);
    m_name = std::move(other.m_name);
    m_cmd_bufs = std::move(other.m_cmd_bufs);
    m_queue_type = other.m_queue_type;
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_device.device(), m_cmd_pool, nullptr);
}

CommandBuffer &CommandPool::request_command_buffer(const std::string &name) const {
    // Try to find a command buffer which is currently not used
    for (const auto &cmd_buf : m_cmd_bufs) {
        if (cmd_buf->m_cmd_buf_execution_completed->status() == VK_SUCCESS) {
            // Reset the command buffer's fence to make it usable again
            cmd_buf->m_cmd_buf_execution_completed->reset_fence();
            cmd_buf->begin_command_buffer();
            m_device.set_debug_name(cmd_buf->m_cmd_buf, name);
            return *cmd_buf;
        }
    }

    spdlog::trace("Creating {} new command buffer #{}", vk_tools::as_string(m_queue_type), 1 + m_cmd_bufs.size());

    // No free command buffer was found so we need to create a new one
    // Note that there is currently no method for shrinking m_cmd_bufs, but this should not be a problem
    m_cmd_bufs.emplace_back(std::make_unique<CommandBuffer>(m_device, m_cmd_pool, name));

    auto &new_cmd_buf = *m_cmd_bufs.back();
    new_cmd_buf.begin_command_buffer();
    m_device.set_debug_name(new_cmd_buf.m_cmd_buf, name);
    return new_cmd_buf;
}

} // namespace inexor::vulkan_renderer::wrapper::commands
