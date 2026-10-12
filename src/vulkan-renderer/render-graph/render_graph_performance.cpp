#include "inexor/vulkan-renderer/render-graph/render_graph_performance.hpp"

#include <algorithm>
#include <array>
#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::render_graph {

void RenderGraphPerformance::record_cpu_frame(const std::uint64_t total, const CpuPhaseDurations &durations) {
    ++m_cpu_frames;
    m_cpu_total_ns += total;
    m_cpu_max_ns = std::max(m_cpu_max_ns, total);
    for (std::size_t i = 0; i < cpu_phase_count; ++i) {
        m_cpu_phase_total_ns[i] += durations[i];
        m_cpu_phase_max_ns[i] = std::max(m_cpu_phase_max_ns[i], durations[i]);
    }
}

void RenderGraphPerformance::record_gpu_frame(const double elapsed, const std::optional<double> gpu_idle_ms) {
    ++m_gpu_frames;
    m_gpu_total_ms += elapsed;
    m_gpu_max_ms = std::max(m_gpu_max_ms, elapsed);
    if (gpu_idle_ms) {
        ++m_gpu_idle_samples;
        m_gpu_idle_total_ms += *gpu_idle_ms;
        m_gpu_idle_max_ms = std::max(m_gpu_idle_max_ms, *gpu_idle_ms);
    }
}

void RenderGraphPerformance::record_queue_submit(const std::uint64_t duration_ns,
                                                 const std::size_t wait_semaphore_count) {
    ++m_queue_submits;
    m_queue_submit_total_ns += duration_ns;
    m_queue_submit_max_ns = std::max(m_queue_submit_max_ns, duration_ns);
    m_semaphore_waits += wait_semaphore_count;
    m_max_semaphore_waits = std::max<std::uint64_t>(m_max_semaphore_waits, wait_semaphore_count);
    if (wait_semaphore_count != 0) {
        ++m_submits_with_semaphore_waits;
    }
}

void RenderGraphPerformance::reset() {
    *this = {};
}

void RenderGraphPerformance::log() const {
    const auto ns_to_ms = [](const std::uint64_t total, const std::uint64_t count) {
        return count == 0 ? 0.0 : static_cast<double>(total) / static_cast<double>(count) / 1'000'000.0;
    };
    spdlog::trace("Performance averages over {} CPU frames:", m_cpu_frames);
    if (m_cpu_frames != 0) {
        spdlog::trace("  CPU frame: {:.3f} ms (max {:.3f} ms)", ns_to_ms(m_cpu_total_ns, m_cpu_frames),
                      static_cast<double>(m_cpu_max_ns) / 1'000'000.0);
        constexpr std::array<const char *, cpu_phase_count> names{"housekeeping", "acquire",     "frame-context",
                                                                  "resources",    "descriptors", "recording",
                                                                  "submit",       "finalize",    "present"};
        for (std::size_t i = 0; i < cpu_phase_count; ++i)
            spdlog::trace("  CPU {:<13}: {:.3f} ms (max {:.3f} ms)", names[i],
                          ns_to_ms(m_cpu_phase_total_ns[i], m_cpu_frames),
                          static_cast<double>(m_cpu_phase_max_ns[i]) / 1'000'000.0);
    }
    if (m_gpu_frames != 0)
        spdlog::trace("  GPU frame: {} samples, avg {:.3f} ms, max {:.3f} ms", m_gpu_frames,
                      m_gpu_total_ms / static_cast<double>(m_gpu_frames), m_gpu_max_ms);
    if (m_queue_submits != 0) {
        spdlog::trace("  Queue submit: count={} avg={:.3f} ms max={:.3f} ms", m_queue_submits,
                      ns_to_ms(m_queue_submit_total_ns, m_queue_submits),
                      static_cast<double>(m_queue_submit_max_ns) / 1'000'000.0);
        spdlog::trace("  Semaphore waits: total={} submits-with-waits={} frequency={:.1f}% max-per-submit={}",
                      m_semaphore_waits, m_submits_with_semaphore_waits,
                      100.0 * static_cast<double>(m_submits_with_semaphore_waits) /
                          static_cast<double>(m_queue_submits),
                      m_max_semaphore_waits);
    }
    if (m_gpu_idle_samples != 0) {
        spdlog::trace("  GPU waiting for CPU (inter-frame idle estimate): samples={} avg={:.3f} ms max={:.3f} ms",
                      m_gpu_idle_samples, m_gpu_idle_total_ms / static_cast<double>(m_gpu_idle_samples),
                      m_gpu_idle_max_ms);
    }
}

} // namespace inexor::vulkan_renderer::render_graph
