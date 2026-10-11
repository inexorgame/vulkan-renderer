#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain_performance.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::wrapper::swapchains {

void SwapchainPerformance::record_acquire(const std::uint64_t d) {
    ++m_statistics.acquires;
    m_statistics.acquire_total_ns += d;
    m_statistics.acquire_max_ns = std::max(m_statistics.acquire_max_ns, d);
}

void SwapchainPerformance::record_present(const std::uint64_t d) {
    ++m_statistics.presents;
    m_statistics.present_total_ns += d;
    m_statistics.present_max_ns = std::max(m_statistics.present_max_ns, d);
}

void SwapchainPerformance::record_image_wait(const std::uint64_t d) {
    ++m_statistics.image_waits;
    m_statistics.image_wait_total_ns += d;
    m_statistics.image_wait_max_ns = std::max(m_statistics.image_wait_max_ns, d);
}

} // namespace inexor::vulkan_renderer::wrapper::swapchains
