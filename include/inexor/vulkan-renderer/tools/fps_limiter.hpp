#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace inexor::vulkan_renderer::tools {

/// A wrapper class for counting and limiting frames per second.
class FPSLimiter {
private:
    std::uint32_t m_max_fps{DEFAULT_FPS};
    std::chrono::milliseconds m_frame_time{DEFAULT_FPS / 1000};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_fps_update_time;
    std::chrono::milliseconds m_fps_update_interval{1000};
    std::uint32_t m_frames{0};

    // The requested max_fps will be clamped in between these limits.
    static constexpr std::uint32_t MIN_FPS{1};
    static constexpr std::uint32_t MAX_FPS{2000};

public:
    static constexpr std::uint32_t DEFAULT_FPS{1000};

    FPSLimiter(std::uint32_t max_fps = DEFAULT_FPS);

    void set_max_fps(std::uint32_t max_fps);

    /// Ask if the next frame is allowed to be rendered.
    [[nodiscard]] bool is_next_frame_allowed();

    /// Return the fps every second, std::nullopt otherwise.
    [[nodiscard]] std::optional<std::uint32_t> get_fps();
};

} // namespace inexor::vulkan_renderer::tools
