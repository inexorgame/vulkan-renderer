#include "inexor/vulkan-renderer/wrapper/synchronization/fence.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <cassert>
#include <limits>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::synchronization {

Fence::Fence(const Device &device, const std::string &name) : m_device(device), m_name(name) {
    if (name.empty()) {
        throw std::invalid_argument("Error: Parameter 'name' is empty!");
    }
    const auto fence_ci = tools::make_info<VkFenceCreateInfo>();
    if (const auto result = vkCreateFence(m_device.device(), &fence_ci, nullptr, &m_fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed!", result, m_name);
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

void Fence::reset() const {
    vkResetFences(m_device.device(), 1, &m_fence);
}

VkResult Fence::status() const {
    return vkGetFenceStatus(m_device.device(), m_fence);
}

void Fence::wait() const {
    if (const auto result =
            vkWaitForFences(m_device.device(), 1, &m_fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkWaitForFences failed!", result, m_name);
    }
}
} // namespace inexor::vulkan_renderer::wrapper::synchronization
