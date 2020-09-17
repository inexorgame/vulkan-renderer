#pragma once

#include <chrono>

namespace inexor::vulkan_renderer {

/// @brief Responsible for calculating the amount of time which has passed between rendering two frames.
/// Since every machine has slightly different speed, it is neccesary to the timestep when animating something.
/// @todo Implement time step for every thread?
class TimeStep {
    // The time point of the last render call.
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;

    // The time point of initialisation.
    std::chrono::time_point<std::chrono::high_resolution_clock> m_initialisation_time;

public:
    TimeStep();

    ~TimeStep() = default;

    /// @brief Returns a scaling factor which corresponds to the
    /// time which has passed since last render call and now.
    [[nodiscard]] float time_step();

    /// @brief Returns a scaling factor which corresponds to the
    /// time which has passed since initialisation and now.
    [[nodiscard]] float time_step_since_initialisation();
};

} // namespace inexor::vulkan_renderer
