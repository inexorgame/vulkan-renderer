#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <cassert>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Fence {
private:
    wrapper::Device &m_device;
    std::string m_name;
    VkFence m_fence;

public:
    /// @brief Creates a new fence.
    /// @param device [in] The Vulkan device.
    /// @param name [in] The internal name of the fence.
    /// @param in_signaled_state [in] If true, the fence will be created in signaled state.
    Fence(wrapper::Device &device, const std::string &name, const bool in_signaled_state);
    Fence(const Fence &) = delete;
    Fence(Fence &&) noexcept;
    ~Fence();

    Fence &operator=(const Fence &) = delete;
    Fence &operator=(Fence &&) = default;

    [[nodiscard]] VkFence get() const {
        assert(m_fence);
        return m_fence;
    }

    ///
    ///
    void block(std::uint64_t timeout_limit = std::numeric_limits<std::uint64_t>::max()) const;

    ///
    void reset() const;
};

} // namespace inexor::vulkan_renderer::wrapper
