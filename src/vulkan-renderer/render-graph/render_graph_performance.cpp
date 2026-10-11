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

void RenderGraphPerformance::record_gpu_frame(const double elapsed) {
    ++m_gpu_frames;
    m_gpu_total_ms += elapsed;
    m_gpu_max_ms = std::max(m_gpu_max_ms, elapsed);
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
}

} // namespace inexor::vulkan_renderer::render_graph
