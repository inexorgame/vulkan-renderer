#include "vulkan-renderer/time-step/time_step.hpp"

namespace inexor {
namespace vulkan_renderer {

InexorTimeStep::InexorTimeStep() {
    initialisation_time = std::chrono::high_resolution_clock::now();

    last_time = std::chrono::high_resolution_clock::now();
}

float InexorTimeStep::get_time_step() {
    auto current_time = std::chrono::high_resolution_clock::now();

    auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();

    last_time = current_time;

    return time_duration;
}

float InexorTimeStep::get_time_step_since_initialisation() {
    auto current_time = std::chrono::high_resolution_clock::now();

    auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - initialisation_time).count();

    return time_duration;
}

}; // namespace vulkan_renderer
}; // namespace inexor
