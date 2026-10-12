#pragma once

#include <volk.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::swapchains {

class SwapchainPerformance {
public:
    enum class RecreationReason { Explicit, AcquireOutOfDate, PresentSuboptimal, PresentOutOfDate };

    struct Statistics {
        std::uint32_t acquires{0}, presents{0};
        std::uint64_t acquire_total_ns{0}, acquire_max_ns{0}, present_total_ns{0}, present_max_ns{0};
        std::uint32_t image_waits{0};
        std::uint64_t image_wait_total_ns{0}, image_wait_max_ns{0};
        std::uint32_t acquire_successes{0}, acquire_suboptimal{0}, acquire_out_of_date{0}, acquire_errors{0};
        std::uint32_t present_successes{0}, present_suboptimal{0}, present_out_of_date{0}, present_errors{0};
        std::uint32_t immediate_acquires{0}, blocking_acquires{0};
        std::uint32_t recreations{0}, explicit_recreations{0}, acquire_out_of_date_recreations{0};
        std::uint32_t present_suboptimal_recreations{0}, present_out_of_date_recreations{0};
        std::uint32_t max_frames_in_flight{0};
        std::uint32_t completed_frames{0}, present_intervals{0};
        std::uint64_t frame_total_ns{0}, frame_max_ns{0};
        std::uint64_t present_interval_total_ns{0}, present_interval_max_ns{0}, last_present_interval_ns{0};
        std::vector<std::uint64_t> image_acquires;

        [[nodiscard]] double acquire_average_ns() const noexcept {
            return average(acquire_total_ns, acquires);
        }
        [[nodiscard]] double present_average_ns() const noexcept {
            return average(present_total_ns, presents);
        }
        [[nodiscard]] double image_wait_average_ns() const noexcept {
            return average(image_wait_total_ns, image_waits);
        }
        [[nodiscard]] double frame_average_ns() const noexcept {
            return average(frame_total_ns, completed_frames);
        }
        [[nodiscard]] double present_interval_average_ns() const noexcept {
            return average(present_interval_total_ns, present_intervals);
        }
        [[nodiscard]] std::uint64_t acquire_percentile_ns(double percentile) const;
        [[nodiscard]] std::uint64_t present_percentile_ns(double percentile) const;
        [[nodiscard]] std::uint64_t image_wait_percentile_ns(double percentile) const;
        [[nodiscard]] std::uint64_t frame_percentile_ns(double percentile) const;
        [[nodiscard]] std::uint64_t present_interval_percentile_ns(double percentile) const;

    private:
        friend class SwapchainPerformance;
        std::vector<std::uint64_t> acquire_samples, present_samples, image_wait_samples, frame_samples;
        std::vector<std::uint64_t> present_interval_samples;

        [[nodiscard]] static double average(const std::uint64_t total_ns, const std::uint32_t count) noexcept {
            return count == 0 ? 0.0 : static_cast<double>(total_ns) / static_cast<double>(count);
        }
        [[nodiscard]] static std::uint64_t percentile_ns(const std::vector<std::uint64_t> &samples, double percentile);
    };
    void record_acquire(std::uint64_t duration_ns, VkResult result);
    void record_image_acquired(std::uint32_t image_index, std::uint32_t frames_in_flight);
    void record_present(std::uint64_t duration_ns, VkResult result);
    void record_image_wait(std::uint64_t duration_ns);
    void record_recreation(RecreationReason reason);
    void configure_images(std::uint32_t image_count);
    void reset() noexcept {
        m_statistics = {};
        m_frame_start.reset();
        m_last_present.reset();
    }
    [[nodiscard]] const Statistics &statistics() const noexcept {
        return m_statistics;
    }

private:
    using Clock = std::chrono::steady_clock;
    Statistics m_statistics;
    std::optional<Clock::time_point> m_frame_start;
    std::optional<Clock::time_point> m_last_present;
};

} // namespace inexor::vulkan_renderer::wrapper::swapchains
