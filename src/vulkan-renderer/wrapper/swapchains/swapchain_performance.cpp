#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain_performance.hpp"

#include <algorithm>
#include <cmath>

namespace inexor::vulkan_renderer::wrapper::swapchains {

namespace {
constexpr std::uint64_t immediate_acquire_threshold_ns = 100'000;

void append_sample(std::vector<std::uint64_t> &samples, const std::uint64_t value) {
    constexpr std::size_t maximum_samples = 4096;
    if (samples.size() == maximum_samples) {
        samples.erase(samples.begin(), samples.begin() + maximum_samples / 4);
    }
    samples.push_back(value);
}
} // namespace

std::uint64_t SwapchainPerformance::Statistics::percentile_ns(const std::vector<std::uint64_t> &samples,
                                                              const double percentile) {
    if (samples.empty())
        return 0;
    auto sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const auto clamped = std::clamp(percentile, 0.0, 1.0);
    const auto index = static_cast<std::size_t>(std::ceil(clamped * static_cast<double>(sorted.size())));
    return sorted[std::max<std::size_t>(1, index) - 1];
}

std::uint64_t SwapchainPerformance::Statistics::acquire_percentile_ns(const double percentile) const {
    return percentile_ns(acquire_samples, percentile);
}

std::uint64_t SwapchainPerformance::Statistics::present_percentile_ns(const double percentile) const {
    return percentile_ns(present_samples, percentile);
}

std::uint64_t SwapchainPerformance::Statistics::image_wait_percentile_ns(const double percentile) const {
    return percentile_ns(image_wait_samples, percentile);
}

std::uint64_t SwapchainPerformance::Statistics::frame_percentile_ns(const double percentile) const {
    return percentile_ns(frame_samples, percentile);
}

std::uint64_t SwapchainPerformance::Statistics::present_interval_percentile_ns(const double percentile) const {
    return percentile_ns(present_interval_samples, percentile);
}

void SwapchainPerformance::record_acquire(const std::uint64_t d, const VkResult result) {
    ++m_statistics.acquires;
    m_statistics.acquire_total_ns += d;
    m_statistics.acquire_max_ns = std::max(m_statistics.acquire_max_ns, d);
    append_sample(m_statistics.acquire_samples, d);
    d < immediate_acquire_threshold_ns ? ++m_statistics.immediate_acquires : ++m_statistics.blocking_acquires;
    if (result == VK_SUCCESS)
        ++m_statistics.acquire_successes;
    else if (result == VK_SUBOPTIMAL_KHR)
        ++m_statistics.acquire_suboptimal;
    else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        ++m_statistics.acquire_out_of_date;
    else
        ++m_statistics.acquire_errors;
}

void SwapchainPerformance::record_image_acquired(const std::uint32_t image_index,
                                                 const std::uint32_t frames_in_flight) {
    if (image_index >= m_statistics.image_acquires.size())
        m_statistics.image_acquires.resize(static_cast<std::size_t>(image_index) + 1);
    ++m_statistics.image_acquires[image_index];
    m_statistics.max_frames_in_flight = std::max(m_statistics.max_frames_in_flight, frames_in_flight);
    m_frame_start = Clock::now();
}

void SwapchainPerformance::record_present(const std::uint64_t d, const VkResult result) {
    ++m_statistics.presents;
    m_statistics.present_total_ns += d;
    m_statistics.present_max_ns = std::max(m_statistics.present_max_ns, d);
    append_sample(m_statistics.present_samples, d);
    if (result == VK_SUCCESS)
        ++m_statistics.present_successes;
    else if (result == VK_SUBOPTIMAL_KHR)
        ++m_statistics.present_suboptimal;
    else if (result == VK_ERROR_OUT_OF_DATE_KHR)
        ++m_statistics.present_out_of_date;
    else
        ++m_statistics.present_errors;

    const auto now = Clock::now();
    if (m_frame_start) {
        const auto frame_ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - *m_frame_start).count());
        ++m_statistics.completed_frames;
        m_statistics.frame_total_ns += frame_ns;
        m_statistics.frame_max_ns = std::max(m_statistics.frame_max_ns, frame_ns);
        append_sample(m_statistics.frame_samples, frame_ns);
        m_frame_start.reset();
    }
    if (m_last_present) {
        const auto interval_ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - *m_last_present).count());
        ++m_statistics.present_intervals;
        m_statistics.last_present_interval_ns = interval_ns;
        m_statistics.present_interval_total_ns += interval_ns;
        m_statistics.present_interval_max_ns = std::max(m_statistics.present_interval_max_ns, interval_ns);
        append_sample(m_statistics.present_interval_samples, interval_ns);
    }
    m_last_present = now;
}

void SwapchainPerformance::record_image_wait(const std::uint64_t d) {
    ++m_statistics.image_waits;
    m_statistics.image_wait_total_ns += d;
    m_statistics.image_wait_max_ns = std::max(m_statistics.image_wait_max_ns, d);
    append_sample(m_statistics.image_wait_samples, d);
}

void SwapchainPerformance::record_recreation(const RecreationReason reason) {
    ++m_statistics.recreations;
    switch (reason) {
    case RecreationReason::Explicit:
        ++m_statistics.explicit_recreations;
        break;
    case RecreationReason::AcquireOutOfDate:
        ++m_statistics.acquire_out_of_date_recreations;
        break;
    case RecreationReason::PresentSuboptimal:
        ++m_statistics.present_suboptimal_recreations;
        break;
    case RecreationReason::PresentOutOfDate:
        ++m_statistics.present_out_of_date_recreations;
        break;
    }
}

void SwapchainPerformance::configure_images(const std::uint32_t image_count) {
    m_statistics.image_acquires.assign(image_count, 0);
}

} // namespace inexor::vulkan_renderer::wrapper::swapchains
