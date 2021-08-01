#include "inexor/vulkan-renderer/wrapper/fence.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Fence::Fence(const wrapper::Device &device, const std::string &name, const bool in_signaled_state)
    : m_device(device), m_name(name) {
    assert(!name.empty());
    assert(device.device());

    auto fence_ci = make_info<VkFenceCreateInfo>();
    fence_ci.flags = in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    spdlog::debug("Creating Vulkan synchronisation fence {}.", m_name);

    if (const auto result = vkCreateFence(device.device(), &fence_ci, nullptr, &m_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, m_name);
}

Fence::Fence(Fence &&other) noexcept : m_device(other.m_device) {
    m_fence = std::exchange(other.m_fence, nullptr);
    m_name = std::move(other.m_name);
}

Fence::~Fence() {
    vkDestroyFence(m_device.device(), m_fence, nullptr);
}

void Fence::block(std::uint64_t timeout_limit) const {
    vkWaitForFences(m_device.device(), 1, &m_fence, VK_TRUE, timeout_limit);
}

void Fence::reset() const {
    vkResetFences(m_device.device(), 1, &m_fence);
}

} // namespace inexor::vulkan_renderer::wrapper
