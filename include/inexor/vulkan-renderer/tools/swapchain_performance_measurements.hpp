#pragma once

#include <cstdint>

namespace inexor::vulkan_renderer::tools {

class SwapchainPerformanceMeasurements {
public:
    struct Statistics {
        std::uint32_t acquires{0};
        std::uint32_t presents{0};
        std::uint64_t acquire_total_ns{0}, acquire_max_ns{0};
        std::uint64_t present_total_ns{0}, present_max_ns{0};
        std::uint32_t image_waits{0};
        std::uint64_t image_wait_total_ns{0}, image_wait_max_ns{0};
    };

    void record_acquire(std::uint64_t duration_ns);
    void record_present(std::uint64_t duration_ns);
    void record_image_wait(std::uint64_t duration_ns);
    void reset() noexcept {
        m_statistics = {};
    }
    [[nodiscard]] const Statistics &statistics() const noexcept {
        return m_statistics;
    }

private:
    Statistics m_statistics;
};

} // namespace inexor::vulkan_renderer::tools
