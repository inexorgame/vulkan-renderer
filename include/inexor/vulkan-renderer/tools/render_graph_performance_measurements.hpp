#pragma once

#include <array>
#include <cstdint>

namespace inexor::vulkan_renderer::tools {

class RenderGraphPerformanceMeasurements {
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
    void record_gpu_frame(double elapsed_ms);
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
};

} // namespace inexor::vulkan_renderer::tools
