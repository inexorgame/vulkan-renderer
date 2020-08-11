#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

OnceCommandBuffer::OnceCommandBuffer(wrapper::Device &device, const VkQueue data_transfer_queue,
                                     const std::uint32_t data_transfer_queue_family_index)
    : m_device(device), m_data_transfer_queue(data_transfer_queue),
      m_command_pool(device.device(), data_transfer_queue_family_index) {

    assert(device.device());
    assert(data_transfer_queue);
    m_command_buffer_created = false;
    m_recording_started = false;
}

OnceCommandBuffer::OnceCommandBuffer(OnceCommandBuffer &&other) noexcept
    : m_device(other.m_device), m_command_pool(std::move(other.m_command_pool)),
      m_command_buffer(std::exchange(other.m_command_buffer, nullptr)),
      m_data_transfer_queue(other.m_data_transfer_queue), m_recording_started(other.m_recording_started),
      m_command_buffer_created(other.m_command_buffer_created) {}

OnceCommandBuffer::~OnceCommandBuffer() {
    m_command_buffer.reset();
    m_command_buffer_created = false;
    m_recording_started = false;
}

void OnceCommandBuffer::create_command_buffer() {
    assert(m_device.device());
    assert(m_command_pool.get());
    assert(m_data_transfer_queue);
    assert(!m_recording_started);
    assert(!m_command_buffer_created);

    m_command_buffer = std::make_unique<wrapper::CommandBuffer>(m_device, m_command_pool.get(), "Once command buffer");

    m_command_buffer_created = true;
}

void OnceCommandBuffer::start_recording() {
    assert(m_device.device());
    assert(m_command_pool.get());
    assert(m_data_transfer_queue);
    assert(m_command_buffer_created);
    assert(!m_recording_started);

    spdlog::debug("Starting recording of once command buffer.");

    auto command_buffer_bi = make_info<VkCommandBufferBeginInfo>();

    // We're only going to use the command buffer once and wait with returning from the function until the copy
    // operation has finished executing. It's good practice to tell the driver about our intent using
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    command_buffer_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_command_buffer->get(), &command_buffer_bi) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkBeginCommandBuffer failed for once command buffer!");
    }

    m_recording_started = true;
}

void OnceCommandBuffer::end_recording_and_submit_command() {
    assert(m_device.device());
    assert(m_command_pool.get());
    assert(m_command_buffer);
    assert(m_data_transfer_queue);
    assert(m_command_buffer_created);
    assert(m_recording_started);

    spdlog::debug("Ending recording of once command buffer.");

    if (vkEndCommandBuffer(m_command_buffer->get()) != VK_SUCCESS) {
        throw std::runtime_error("Error: VkEndCommandBuffer failed for once command buffer!");
    }

    spdlog::debug("Command buffer recording ended successfully.");

    spdlog::debug("Starting to submit command.");

    auto submit_info = make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = m_command_buffer->ptr();

    if (vkQueueSubmit(m_data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueSubmit failed for once command buffer!");
    }

    // TODO: Refactor! Introduce proper synchronisation using VkFence!
    if (vkQueueWaitIdle(m_data_transfer_queue) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkQueueWaitIdle failed for once command buffer!");
    }

    spdlog::debug("Destroying once command buffer.");

    // Because we destroy the command buffer after submission, we have to allocate it every time.
    vkFreeCommandBuffers(m_device.device(), m_command_pool.get(), 1, m_command_buffer->ptr());

    m_command_buffer_created = false;

    m_recording_started = false;
}

} // namespace inexor::vulkan_renderer::wrapper
