#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace inexor::vulkan_renderer {

class FPSCounter {
private:
    std::size_t m_frames = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;

    float m_fps_update_interval = 1.0f;

public:
    FPSCounter() = default;

    ~FPSCounter() = default;

    std::optional<std::uint32_t> update();
};

} // namespace inexor::vulkan_renderer
