#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <limits>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Fence {
private:
    VkFence m_fence;
    VkDevice m_device;
    std::string m_name;

public:
    Fence(const Fence &) = default;
    Fence(Fence &&other) noexcept;

    Fence &operator=(const Fence &other) = default;
    Fence &operator=(Fence &&other) noexcept = default;

    /// @brief Creates a new fence.
    /// @param device [in] The Vulkan device.
    /// @param name [in] The internal name of the fence.
    /// @param in_signaled_state [in] If true, the fence will be created in signaled state.
    Fence(const VkDevice device, const std::string &name, const bool in_signaled_state);

    ~Fence();

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
