#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    : command_buffer(std::exchange(other.command_buffer, nullptr)) {}

CommandBuffer::CommandBuffer(const VkDevice device, const VkCommandPool command_pool) {
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateCommandBuffers failed for once command buffer!");
    }
}

} // namespace inexor::vulkan_renderer::wrapper
