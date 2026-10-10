#pragma once

#include <volk.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::core::Device;

class FrameSyncManager {
public:
    struct WaitStats {
        std::uint64_t waits{0};
        std::uint64_t total_ns{0};
        std::uint64_t max_ns{0};
    };

private:
    const Device &m_device;
    std::vector<VkFence> m_frame_slot_submission_fences{VK_NULL_HANDLE};
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    struct DeferredRelease {
        std::vector<VkFence> fences;
        std::function<void()> release;
    };

    std::vector<DeferredRelease> m_deferred_releases;
    WaitStats m_wait_stats;

public:
    explicit FrameSyncManager(const Device &device);

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

    /// Wait until the current frame slot is no longer used by a previous submission.
    [[nodiscard]] VkFence wait_for_current_frame_slot();

    void mark_frame_slot_submission_fence(VkFence fence);

    void defer_release(std::span<const VkFence> fences, std::function<void()> release);

    void defer_release(VkFence fence, std::function<void()> release);

    void process_deferred_releases(bool wait_all);

    void clear();

    [[nodiscard]] const std::vector<VkFence> &frame_slot_submission_fences() const {
        return m_frame_slot_submission_fences;
    }

    [[nodiscard]] VkFence frame_slot_submission_fence(std::size_t frame_slot) const;

    [[nodiscard]] const WaitStats &wait_stats() const {
        return m_wait_stats;
    }

    [[nodiscard]] std::size_t frame_slot_count() const {
        return m_frame_slot_count;
    }

    [[nodiscard]] std::size_t current_frame_slot() const {
        return m_current_frame_slot;
    }
};

} // namespace inexor::vulkan_renderer::render_graph