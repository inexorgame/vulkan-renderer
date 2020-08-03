#include "inexor/vulkan-renderer/time_step.hpp"

namespace inexor::vulkan_renderer {

TimeStep::TimeStep() {
    m_initialisation_time = std::chrono::high_resolution_clock::now();

    m_last_time = std::chrono::high_resolution_clock::now();
}

float TimeStep::time_step() {
    auto current_time = std::chrono::high_resolution_clock::now();

    auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - m_last_time).count();

    m_last_time = current_time;

    return time_duration;
}

float TimeStep::time_step_since_initialisation() {
    auto current_time = std::chrono::high_resolution_clock::now();

    auto time_duration =
        std::chrono::duration<float, std::chrono::seconds::period>(current_time - m_initialisation_time).count();

    return time_duration;
}

} // namespace inexor::vulkan_renderer
