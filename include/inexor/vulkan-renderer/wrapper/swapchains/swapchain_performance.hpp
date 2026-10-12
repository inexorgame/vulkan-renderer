#pragma once

#include <cstdint>

namespace inexor::vulkan_renderer::wrapper::swapchains {

class SwapchainPerformance {
public:
    struct Statistics {
        std::uint32_t acquires{0}, presents{0};
        std::uint64_t acquire_total_ns{0}, acquire_max_ns{0}, present_total_ns{0}, present_max_ns{0};
        std::uint32_t image_waits{0};
        std::uint64_t image_wait_total_ns{0}, image_wait_max_ns{0};

        [[nodiscard]] double acquire_average_ns() const noexcept {
            return average(acquire_total_ns, acquires);
        }
        [[nodiscard]] double present_average_ns() const noexcept {
            return average(present_total_ns, presents);
        }
        [[nodiscard]] double image_wait_average_ns() const noexcept {
            return average(image_wait_total_ns, image_waits);
        }

    private:
        [[nodiscard]] static double average(const std::uint64_t total_ns, const std::uint32_t count) noexcept {
            return count == 0 ? 0.0 : static_cast<double>(total_ns) / static_cast<double>(count);
        }
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

} // namespace inexor::vulkan_renderer::wrapper::swapchains
