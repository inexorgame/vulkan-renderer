#include "example_app_performance.hpp"

#include <algorithm>

namespace inexor::example_app {

void ExampleAppPerformance::push(const float cpu, const std::optional<float> gpu) {
    constexpr std::size_t average_window = 10;
    const auto count = std::min<std::size_t>(m_history_count + 1, average_window);
    const auto average = [&](const auto &history, const float fallback) {
        float sum = 0.0f;
        for (std::size_t i = 0; i < count; ++i)
            sum += history[(m_history_index + history_size - i) % history_size];
        return count == 0 ? fallback : sum / static_cast<float>(count);
    };
    m_cpu_raw_samples[m_history_index] = cpu;
    m_last_cpu = cpu;
    m_cpu_samples[m_history_index] = average(m_cpu_raw_samples, cpu);
    if (gpu) {
        m_last_gpu = gpu;
        m_gpu_raw_samples[m_history_index] = *gpu;
    } else if (m_history_count > 0)
        m_gpu_raw_samples[m_history_index] = m_gpu_raw_samples[(m_history_index + history_size - 1) % history_size];
    else {
        m_last_gpu.reset();
        m_gpu_raw_samples[m_history_index] = cpu;
    }
    m_gpu_samples[m_history_index] = average(m_gpu_raw_samples, m_gpu_raw_samples[m_history_index]);
    m_history_index = (m_history_index + 1) % history_size;
    if (m_history_count < history_size)
        ++m_history_count;
}

void ExampleAppPerformance::update(const float cpu, const std::optional<float> gpu, const double elapsed) {
    m_last_cpu = cpu;
    if (gpu)
        m_last_gpu = gpu;
    m_accumulator += elapsed;
    while (m_accumulator >= 0.01) {
        push(m_last_cpu, m_last_gpu);
        m_accumulator -= 0.01;
    }
}

} // namespace inexor::example_app
