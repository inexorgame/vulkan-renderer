#pragma once

#include <array>
#include <cstddef>
#include <optional>

namespace inexor::vulkan_renderer::tools {

class ApplicationPerformanceMeasurements {
public:
    static constexpr std::size_t history_size = 240;
    static constexpr std::size_t average_window = 10;

    void update(float cpu_frame_time_ms, std::optional<float> gpu_frame_time_ms, double elapsed_seconds);
    [[nodiscard]] std::size_t sample_count() const noexcept {
        return m_history_count;
    }
    [[nodiscard]] std::size_t sample_index() const noexcept {
        return m_history_index;
    }
    [[nodiscard]] const std::array<float, history_size> &cpu_samples() const noexcept {
        return m_cpu_samples;
    }
    [[nodiscard]] const std::array<float, history_size> &gpu_samples() const noexcept {
        return m_gpu_samples;
    }

private:
    void push(float cpu_frame_time_ms, std::optional<float> gpu_frame_time_ms);
    std::array<float, history_size> m_cpu_samples{};
    std::array<float, history_size> m_gpu_samples{};
    std::array<float, history_size> m_cpu_raw_samples{};
    std::array<float, history_size> m_gpu_raw_samples{};
    std::size_t m_history_index{0};
    std::size_t m_history_count{0};
    float m_last_cpu{0.0f};
    std::optional<float> m_last_gpu;
    double m_accumulator{0.0};
};

} // namespace inexor::vulkan_renderer::tools
