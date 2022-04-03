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
    CommandBuffer::begin_command_buffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    command_lambda(*this);

    CommandBuffer::flush_command_buffer_and_wait();
}

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : CommandBuffer(std::move(other)), m_command_pool(std::move(other.m_command_pool)) {}

} // namespace inexor::vulkan_renderer::wrapper
