#include "inexor/vulkan-renderer/wrapper/synchronization/fence.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/vulkan_exception.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::synchronization {

Fence::Fence(const Device &device, const std::string &name, const bool in_signaled_state)
    : m_device(device), m_name(name) {
    assert(!name.empty());

    const auto fence_ci = make_info<VkFenceCreateInfo>({
        .flags = static_cast<VkFenceCreateFlags>(in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0),
    });

    if (const auto result = vkCreateFence(m_device.device(), &fence_ci, nullptr, &m_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed for fence " + name + "!", result);
    }
    m_device.set_debug_name(m_fence, m_name);
}

Fence::Fence(Fence &&other) noexcept : m_device(other.m_device) {
    m_fence = std::exchange(other.m_fence, nullptr);
    m_name = std::move(other.m_name);
}

Fence::~Fence() {
    vkDestroyFence(m_device.device(), m_fence, nullptr);
}

void Fence::wait(const std::uint64_t timeout_limit) const {
    // TODO: Error handling!
    vkWaitForFences(m_device.device(), 1, &m_fence, VK_TRUE, timeout_limit);
}

void Fence::reset_fence() const {
    // TODO: Error handling!
    vkResetFences(m_device.device(), 1, &m_fence);
}

VkResult Fence::status() const {
    // TODO: Error handling!
    return vkGetFenceStatus(m_device.device(), m_fence);
}

} // namespace inexor::vulkan_renderer::wrapper::synchronization
