#pragma once

#include <volk.h>

#include <cstdint>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::synchronization {

/// @brief A RAII wrapper for VkFences.
class Fence {
    const Device &m_device;
    std::string m_name;
    VkFence m_fence{VK_NULL_HANDLE};

public:
    /// @brief Default constructor.
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

    [[nodiscard]] auto fence() const {
        return m_fence;
    }

    /// @brief Block fence by calling vkWaitForFences and wait until fence condition is fulfilled.
    /// @param timeout_limit The time to wait in milliseconds. If no time is specified, the numeric maximum value
    /// is used.
    void block(std::uint64_t timeout_limit = std::numeric_limits<std::uint64_t>::max()) const;

    /// @brief Call vkResetFences.
    void reset() const;

    /// Call vkGetFenceStatus
    [[nodiscard]] VkResult status() const;
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
