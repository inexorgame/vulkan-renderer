#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
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

    // TODO: Move into wrapper!
    VkFence wait_fence;
    VkFenceCreateInfo fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr};

    if (const auto result = vkCreateFence(device.device(), &fenceCI, nullptr, &wait_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: VkCreateFence failed!!", result);
    }

    auto submit_info = make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer;

    // TODO: Implement wrapper for queues!
    if (const auto result = vkQueueSubmit(queue, 1, &submit_info, wait_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueSubmit failed for once command buffer!", result);
    }

    // TODO: Refactor! Introduce proper synchronisation using VkFence!
    /* if (const auto result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueWaitIdle failed for once command buffer!", result);
    }
    */

    if (const auto result = vkWaitForFences(device.device(), 1, &wait_fence, VK_TRUE, UINT64_MAX);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkWaitForFences failed!!", result);
    }

    vkDestroyFence(device.device(), wait_fence, nullptr);

    // TODO: vkFreeCommandBuffers?
}

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : CommandBuffer(std::move(other)), m_command_pool(std::move(other.m_command_pool)) {}

} // namespace inexor::vulkan_renderer::wrapper
