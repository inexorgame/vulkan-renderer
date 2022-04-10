#include "inexor/vulkan-renderer/wrapper/fence.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Fence::Fence(const wrapper::Device &device, const std::string &name, const bool in_signaled_state)
    : m_device(device), m_name(name) {
    assert(!name.empty());
    assert(device.device());

    auto fence_ci = make_info<VkFenceCreateInfo>();
    // TODO: Move into make_info
    fence_ci.flags = in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    spdlog::trace("Creating Vulkan synchronisation fence {}.", m_name);

    if (const auto result = vkCreateFence(device.device(), &fence_ci, nullptr, &m_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, m_name);
}

Fence::Fence(const wrapper::Device &device, const std::string &name, std::function<void(const VkFence fence)> command,
             const std::uint64_t timeout_limit, bool in_signaled_state)
    : Fence(device, name, in_signaled_state) {

    // Execute the command and wait
    command(m_fence);
    wait(timeout_limit);
}

Fence::Fence(Fence &&other) noexcept : m_device(other.m_device) {
    m_fence = std::exchange(other.m_fence, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Fence::~Fence() {
    vkDestroyFence(m_device.device(), m_fence, nullptr);
}

void Fence::wait(const std::uint64_t timeout_limit) const {
    vkWaitForFences(m_device.device(), 1, &m_fence, VK_TRUE, timeout_limit);
}

void Fence::reset() const {
    vkResetFences(m_device.device(), 1, &m_fence);
}

} // namespace inexor::vulkan_renderer::wrapper
