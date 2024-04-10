#include "inexor/vulkan-renderer/wrapper/fence.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Fence::Fence(const Device &device, const std::string &name, const bool in_signaled_state)
    : m_device(device), m_name(name) {
    assert(!name.empty());

    const auto fence_ci = make_info<VkFenceCreateInfo>({
        .flags = static_cast<VkFenceCreateFlags>(in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0),
    });

    if (const auto result = vkCreateFence(m_device.device(), &fence_ci, nullptr, &m_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed for fence " + name + "!", result);
    }
    // Set an internal debug name to this fence using Vulkan debug utils (VK_EXT_debug_utils)
    m_device.set_debug_utils_object_name(VK_OBJECT_TYPE_FENCE, reinterpret_cast<std::uint64_t>(m_fence), m_name);
}

Fence::Fence(Fence &&other) noexcept : m_device(other.m_device) {
    m_fence = std::exchange(other.m_fence, nullptr);
    m_name = std::move(other.m_name);
}

Fence::~Fence() {
    vkDestroyFence(m_device.device(), m_fence, nullptr);
}

void Fence::block(const std::uint64_t timeout_limit) const {
    vkWaitForFences(m_device.device(), 1, &m_fence, VK_TRUE, timeout_limit);
}

void Fence::reset() const {
    vkResetFences(m_device.device(), 1, &m_fence);
}

VkResult Fence::status() const {
    return vkGetFenceStatus(m_device.device(), m_fence);
}

} // namespace inexor::vulkan_renderer::wrapper
