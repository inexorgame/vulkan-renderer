#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

OnceCommandBuffer::OnceCommandBuffer(const Device &device, const VkQueue queue, const std::uint32_t queue_family_index)
    : m_device(device), m_queue(queue), m_command_pool(device, queue_family_index) {
    assert(device.device());
    m_recording_started = false;
}

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : m_device(other.m_device), m_command_pool(std::move(other.m_command_pool)) {
    m_queue = other.m_queue;
    m_command_buffer = std::exchange(other.m_command_buffer, nullptr);
    m_recording_started = other.m_recording_started;
}

OnceCommandBuffer::~OnceCommandBuffer() {
    m_command_buffer.reset();
    m_recording_started = false;
}

void OnceCommandBuffer::create_command_buffer() {
    assert(m_command_pool.get());
    assert(!m_recording_started);
    assert(!m_command_buffer);

    m_command_buffer = std::make_unique<wrapper::CommandBuffer>(m_device, m_command_pool.get(), "Once command buffer");
}

void OnceCommandBuffer::start_recording() {
    assert(m_command_pool.get());
    assert(!m_recording_started);

    spdlog::trace("Starting recording of once command buffer.");

    auto command_buffer_bi = make_info<VkCommandBufferBeginInfo>();

    // We're only going to use the command buffer once and wait with returning from the function until the copy
    // operation has finished executing. It's good practice to tell the driver about our intent using
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    command_buffer_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (const auto result = vkBeginCommandBuffer(m_command_buffer->get(), &command_buffer_bi); result != VK_SUCCESS) {
        throw VulkanException("Error: vkBeginCommandBuffer failed for once command buffer!", result);
    }

    m_recording_started = true;
}

void OnceCommandBuffer::end_recording_and_submit_command() {
    assert(m_command_pool.get());
    assert(m_command_buffer);
    assert(m_recording_started);

    spdlog::trace("Ending recording of once command buffer.");

    if (vkEndCommandBuffer(m_command_buffer->get()) != VK_SUCCESS) {
        throw std::runtime_error("Error: VkEndCommandBuffer failed for once command buffer!");
    }

    spdlog::trace("Starting to submit command.");

    auto submit_info = make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = m_command_buffer->ptr();

    if (const auto result = vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueSubmit failed for once command buffer!", result);
    }

    // TODO: Refactor! Introduce proper synchronisation using VkFence!
    if (const auto result = vkQueueWaitIdle(m_queue); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueueWaitIdle failed for once command buffer!", result);
    }

    spdlog::trace("Destroying once command buffer.");

    // Because we destroy the command buffer after submission, we have to allocate it every time.
    vkFreeCommandBuffers(m_device.device(), m_command_pool.get(), 1, m_command_buffer->ptr());

    m_recording_started = false;
}

} // namespace inexor::vulkan_renderer::wrapper
