#include "inexor/vulkan-renderer/wrapper/fence.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

Fence::Fence(Fence &&other) noexcept
    : device(other.device), fence(std::exchange(other.fence, nullptr)), name(std::move(other.name)) {}

Fence::Fence(const VkDevice device, const std::string &name, const bool in_signaled_state)
    : device(device), name(name) {
    assert(device);
    assert(!name.empty());

    VkFenceCreateInfo fence_ci = {};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = in_signaled_state ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    spdlog::debug("Creating Vulkan synchronisation fence {}.", name);

    if (vkCreateFence(device, &fence_ci, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateFence failed!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    spdlog::debug("Created fence successfully.");
}

Fence::~Fence() {
    spdlog::trace("Destroying fence {}.", name);
    vkDestroyFence(device, fence, nullptr);
}

void Fence::block(std::uint64_t timeout_limit) const {
    assert(device);
    assert(fence);
    vkWaitForFences(device, 1, &fence, VK_TRUE, timeout_limit);
}

void Fence::reset() const {
    assert(device);
    assert(fence);
    vkResetFences(device, 1, &fence);
}

} // namespace inexor::vulkan_renderer::wrapper
