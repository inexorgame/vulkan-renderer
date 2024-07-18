#pragma once

#include <volk.h>

#include <cstdint>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::synchronization {

// Using declaration
using wrapper::Device;
using wrapper::commands::CommandBuffer;

/// A RAII wrapper for VkFence
class Fence {
    friend class CommandBuffer;

private:
    const Device &m_device;
    std::string m_name;
    VkFence m_fence{VK_NULL_HANDLE};

public:
    /// Default constructor
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the VkFence.
    /// @param in_signaled_state True if the VkFence will be constructed in signaled state, false otherwise.
    /// @warning Make sure to specify in_signaled_state correctly as needed, otherwise synchronization problems occur.
    Fence(const Device &device, const std::string &name, bool in_signaled_state);

    Fence(const Fence &) = delete;
    Fence(Fence &&) noexcept;

    ~Fence();

    Fence &operator=(const Fence &) = delete;
    Fence &operator=(Fence &&) = delete;

    /// Call vkCmdWaitForFences
    /// @param timeout_limit The time to wait in milliseconds (numeric limit by default)
    void wait(std::uint64_t timeout_limit = std::numeric_limits<std::uint64_t>::max()) const;

    /// Call vkResetFences
    /// @note This is deliberately called reset_fences and not reset because it is easy to confuse this with the reset
    /// method of smart pointers, which could end up in a lot of bugs.
    void reset_fence() const;

    /// Call vkGetFenceStatus
    [[nodiscard]] VkResult status() const;
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
