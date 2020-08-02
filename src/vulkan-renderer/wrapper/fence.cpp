#include "inexor/vulkan-renderer/wrapper/fence.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

Fence::Fence(Fence &&other) noexcept
    : m_device(other.m_device), m_fence(std::exchange(other.m_fence, nullptr)), m_name(std::move(other.m_name)) {}

Fence::Fence(const VkDevice device, const std::string &name, const bool in_signaled_state)
    : m_device(device), m_name(name) {
    assert(device);
    assert(!name.empty());

    auto fence_ci = make_info<VkFenceCreateInfo>();
    fence_ci.flags = in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    spdlog::debug("Creating Vulkan synchronisation fence {}.", name);

    if (vkCreateFence(device, &fence_ci, nullptr, &m_fence) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateFence failed!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    spdlog::debug("Created fence successfully.");
}

Fence::~Fence() {
    spdlog::trace("Destroying fence {}.", m_name);
    vkDestroyFence(m_device, m_fence, nullptr);
}

void Fence::block(std::uint64_t timeout_limit) const {
    assert(m_device);
    assert(m_fence);
    vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout_limit);
}

void Fence::reset() const {
    assert(m_device);
    assert(m_fence);
    vkResetFences(m_device, 1, &m_fence);
}

} // namespace inexor::vulkan_renderer::wrapper
