#pragma once

#include <volk.h>

#include <cstdint>
#include <string>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::synchronization {

// Using declaration

/// A RAII wrapper for VkFence
class Fence {
private:
    const core::Device &m_device;
    std::string m_name;
    VkFence m_fence{VK_NULL_HANDLE};

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the VkFence.
    Fence(const core::Device &device, const std::string &name);

    Fence(const Fence &) = delete;
    Fence(Fence &&) noexcept;

    ~Fence();

    Fence &operator=(const Fence &) = delete;
    Fence &operator=(Fence &&) = delete;

    /// Call vkCmdWaitForFences
    void wait() const;

    [[nodiscard]] auto fence() const {
        return m_fence;
    }

    /// Call vkResetFences
    void reset() const;

    /// Call vkGetFenceStatus
    [[nodiscard]] VkResult status() const;
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
