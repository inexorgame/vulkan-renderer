#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class Fence {
    const wrapper::Device &m_device;
    const std::string m_name;
    VkFence m_fence;

public:
    /// @brief Creates a new fence.
    /// @param device [in] The Vulkan device.
    /// @param name [in] The internal name of the fence.
    /// @param in_signaled_state [in] If true, the fence will be created in signaled state.
    Fence(const wrapper::Device &device, const std::string &name, const bool in_signaled_state);
    Fence(const Fence &) = delete;
    Fence(Fence &&) noexcept;
    ~Fence();

    Fence &operator=(const Fence &) = delete;
    Fence &operator=(Fence &&) = default;

    [[nodiscard]] VkFence get() const {
        return m_fence;
    }

    /// @brief Blocks fence and waits until fence condition is fulfilled.
    /// @param timeout_limit [in] The maximum time to wait in milliseconds.
    void block(std::uint64_t timeout_limit = std::numeric_limits<std::uint64_t>::max()) const;

    /// @brief Resets the fence.
    void reset() const;
};

} // namespace inexor::vulkan_renderer::wrapper
