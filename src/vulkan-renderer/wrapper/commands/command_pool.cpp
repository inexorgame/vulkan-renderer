#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <spdlog/spdlog.h>

#include <sstream>
#include <thread>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

CommandPool::CommandPool(const core::Device &device, const VkQueueFlagBits queue_type,
                         const std::uint32_t queue_family_index, std::string name)
    : m_device(device), m_name(std::move(name)), m_queue_type(queue_type) {

    const auto cmd_pool_ci = tools::make_info<VkCommandPoolCreateInfo>({
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = queue_family_index,
    });

    // Converting the thread ID to std::string is more complicated than you might think, we need a stringstream.
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    spdlog::trace("Creating command pool [thread={}, type={}]", oss.str(), tools::as_string(queue_type));

    if (const auto result = vkCreateCommandPool(m_device.device(), &cmd_pool_ci, nullptr, &m_cmd_pool);
        result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkCreateCommandPool failed!", result, m_name);
    }
    m_device.set_debug_name(m_cmd_pool, m_name);
}

CommandPool::CommandPool(CommandPool &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_cmd_pool = std::exchange(other.m_cmd_pool, nullptr);
    m_queue_type = other.m_queue_type;
    m_cmd_bufs = std::move(other.m_cmd_bufs);
    m_secondary_cmd_bufs = std::move(other.m_secondary_cmd_bufs);
    m_next_reuse_index = other.m_next_reuse_index;
    m_next_secondary_reuse_index = other.m_next_secondary_reuse_index;
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_device.device(), m_cmd_pool, nullptr);
}

std::string CommandPool::get_pool_name(const VkQueueFlagBits queue_type) const {
    switch (queue_type) {
    case VK_QUEUE_GRAPHICS_BIT:
        return "graphics";
    case VK_QUEUE_COMPUTE_BIT:
        return "compute";
    case VK_QUEUE_TRANSFER_BIT:
        return "transfer";
    case VK_QUEUE_SPARSE_BINDING_BIT:
        return "sparse binding";
    default:
        return "unknown queue type";
    }
}

const CommandBuffer &CommandPool::request_command_buffer(const std::string &name) {
    // Allocate additional command buffers only up to the configured in-flight limit.
    if (m_cmd_bufs.size() < MAX_IN_FLIGHT_SUBMISSIONS) {
        const auto cmd_buffer_number = std::to_string(m_cmd_bufs.size());
        const auto cmd_pool_name = get_pool_name(m_queue_type);
        std::string cmd_buf_name = "[" + cmd_pool_name + "] Command Buffer " + cmd_buffer_number;
        m_cmd_bufs.emplace_back(std::make_unique<CommandBuffer>(m_device, m_cmd_pool, cmd_buf_name));
        spdlog::trace("Creating command buffer [type=primary, index={}, pool={}]", cmd_buffer_number, cmd_pool_name);
        m_cmd_bufs.back()->set_debug_name(name);
        m_cmd_bufs.back()->begin_command_buffer();
        return *m_cmd_bufs.back();
    }

    // All command buffers are currently in-flight: wait for one slot and reuse it.
    auto &cmd_buf = m_cmd_bufs[m_next_reuse_index];
    cmd_buf->wait_fence();
    cmd_buf->reset_fence();
    cmd_buf->set_debug_name(name);
    cmd_buf->reset();
    cmd_buf->begin_command_buffer();
    m_next_reuse_index = (m_next_reuse_index + 1) % m_cmd_bufs.size();
    return *cmd_buf;
}

const CommandBuffer &CommandPool::request_secondary_command_buffer(const std::string &name) {
    for (auto &cmd_buf : m_secondary_cmd_bufs) {
        if (cmd_buf.name() == name) {
            cmd_buf.set_debug_name(name);
            return cmd_buf;
        }
    }

    // Create a dedicated secondary command buffer for each unique pass name.
    const auto cmd_buffer_number = std::to_string(m_secondary_cmd_bufs.size());
    const auto cmd_pool_name = get_pool_name(m_queue_type);
    std::string cmd_buf_name = "[" + cmd_pool_name + "] Secondary Command Buffer " + cmd_buffer_number;
    m_secondary_cmd_bufs.emplace_back(m_device, m_cmd_pool, cmd_buf_name, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    spdlog::trace("Creating command buffer [type=secondary, index={}, pool={}]", cmd_buffer_number, cmd_pool_name);
    m_secondary_cmd_bufs.back().set_debug_name(name);
    return m_secondary_cmd_bufs.back();
}

void CommandPool::wait_for_all_submissions() const {
    for (const auto &cmd_buf : m_cmd_bufs) {
        if (cmd_buf->was_submitted()) {
            cmd_buf->wait_fence();
        }
    }
}

} // namespace inexor::vulkan_renderer::wrapper::commands
