#include "inexor/vulkan-renderer/command_buffer.hpp"

namespace inexor::vulkan_renderer {

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : name(std::move(other.name)), command_buffer(std::exchange(other.command_buffer, nullptr)) {}

CommandBuffer::CommandBuffer(const VkDevice &device, const std::string &name, const VkCommandPool &command_pool) : name(name) {
    assert(device);
    assert(!name.empty());
    assert(command_pool);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

    // TODO: Add more parameters to constructor!
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    spdlog::debug("Allocating command buffers for mesh buffer manager.");

    if (vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateCommandBuffers failed for " + name + " !");
    }

    // TODO: Set Vulkan debug marker name!
}

CommandBuffer::~CommandBuffer() {
    // We don't have to explicitely destroy command buffers since
    // they will be destroyed with the corresponding command pool.
}

} // namespace inexor::vulkan_renderer
