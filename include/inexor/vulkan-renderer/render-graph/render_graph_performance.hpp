#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace inexor::vulkan_renderer::render_graph {

class RenderGraphPerformance {
public:
    enum class CpuPhase : std::size_t {
        Housekeeping,
        Acquire,
        FrameContext,
        Resources,
        Descriptors,
        Recording,
        Submit,
        Finalize,
        Present,
        Count
    };
    static constexpr std::size_t cpu_phase_count = static_cast<std::size_t>(CpuPhase::Count);
    using CpuPhaseDurations = std::array<std::uint64_t, cpu_phase_count>;

    void record_cpu_frame(std::uint64_t total_ns, const CpuPhaseDurations &durations);
    void record_gpu_frame(double elapsed_ms, std::optional<double> gpu_idle_ms = std::nullopt);
    void record_queue_submit(std::uint64_t duration_ns, std::size_t wait_semaphore_count);
    void reset();
    void log() const;

private:
    std::uint32_t m_cpu_frames{0};
    std::uint64_t m_cpu_total_ns{0};
    std::uint64_t m_cpu_max_ns{0};
    CpuPhaseDurations m_cpu_phase_total_ns{};
    CpuPhaseDurations m_cpu_phase_max_ns{};
    std::uint64_t m_gpu_frames{0};
    double m_gpu_total_ms{0.0};
    double m_gpu_max_ms{0.0};
    std::uint64_t m_queue_submits{0};
    std::uint64_t m_queue_submit_total_ns{0}, m_queue_submit_max_ns{0};
    std::uint64_t m_submits_with_semaphore_waits{0}, m_semaphore_waits{0}, m_max_semaphore_waits{0};
    std::uint64_t m_gpu_idle_samples{0};
    double m_gpu_idle_total_ms{0.0}, m_gpu_idle_max_ms{0.0};
};

} // namespace inexor::vulkan_renderer::render_graph
