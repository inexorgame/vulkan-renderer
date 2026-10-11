#include "inexor/vulkan-renderer/tools/swapchain_performance_measurements.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::tools {

void SwapchainPerformanceMeasurements::record_acquire(const std::uint64_t duration) {
    ++m_statistics.acquires;
    m_statistics.acquire_total_ns += duration;
    m_statistics.acquire_max_ns = std::max(m_statistics.acquire_max_ns, duration);
}

void SwapchainPerformanceMeasurements::record_present(const std::uint64_t duration) {
    ++m_statistics.presents;
    m_statistics.present_total_ns += duration;
    m_statistics.present_max_ns = std::max(m_statistics.present_max_ns, duration);
}

void SwapchainPerformanceMeasurements::record_image_wait(const std::uint64_t duration) {
    ++m_statistics.image_waits;
    m_statistics.image_wait_total_ns += duration;
    m_statistics.image_wait_max_ns = std::max(m_statistics.image_wait_max_ns, duration);
}

} // namespace inexor::vulkan_renderer::tools
