#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

OnceCommandBuffer::OnceCommandBuffer(const Device &device,
                                     std::function<void(const CommandBuffer &cmd_buf)> command_lambda)
    : OnceCommandBuffer(device, device.graphics_queue(), device.graphics_queue_family_index(), command_lambda) {}

OnceCommandBuffer::OnceCommandBuffer(const Device &device, const VkQueue queue, const std::uint32_t queue_family_index,
                                     std::function<void(const CommandBuffer &cmd_buf)> command_lambda)
    : m_command_pool(device, device.graphics_queue_family_index()), CommandBuffer(device) {

    CommandBuffer::create_command_buffer(m_command_pool.get());
    CommandBuffer::begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // We need to create a temporary value so we can create a const reference
    const CommandBuffer &cmd_buf = *this;
    command_lambda(cmd_buf);

    CommandBuffer::end();

    auto submit_info = make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer;

    // TODO: Implement wrapper for queues!
    if (const auto result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueSubmit failed for once command buffer!", result);
    }

    // TODO: Refactor! Introduce proper synchronisation using VkFence!
    if (const auto result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueWaitIdle failed for once command buffer!", result);
    }

    // Because we destroy the command buffer after submission, we have to allocate it every time.
    vkFreeCommandBuffers(m_device.device(), m_command_pool.get(), 1, &m_command_buffer);
}

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : CommandBuffer(std::move(other)), m_command_pool(std::move(other.m_command_pool)) {}

} // namespace inexor::vulkan_renderer::wrapper
