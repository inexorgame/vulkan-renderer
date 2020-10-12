#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief A RAII wrapper for VkFences.
class Fence {
    const wrapper::Device &m_device;
    const std::string m_name;
    VkFence m_fence;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the VkFence.
    /// @param in_signaled_state True if the VkFence will be constructored in signaled state, false otherwise.
    /// @warning Make sure to specify in_signaled_state correctly as needed, otherwise synchronization problems occur.
    Fence(const wrapper::Device &device, const std::string &name, const bool in_signaled_state);

    Fence(const Fence &) = delete;
    Fence(Fence &&) noexcept;

    ~Fence();

    Fence &operator=(const Fence &) = delete;
    Fence &operator=(Fence &&) = default;

    [[nodiscard]] VkFence get() const {
        return m_fence;
    }

    /// @brief Block fence by calling vkWaitForFences and wait until fence condition is fulfilled.
    /// @param timeout_limit The time to wait in milliseconds. If no time is specified, the numeric maximum value
    /// is used.
    void block(std::uint64_t timeout_limit = std::numeric_limits<std::uint64_t>::max()) const;

    /// @brief Call vkResetFences.
    void reset() const;
};

} // namespace inexor::vulkan_renderer::wrapper
