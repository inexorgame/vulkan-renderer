#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace inexor::vulkan_renderer::tools {

/// Counts rendered frames and limits the maximum frame rate.
///
/// Call is_next_frame_allowed() before rendering a frame.
/// Call get_fps() exactly once per rendered frame.
class FPSLimiter {
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double>;

    static constexpr std::uint32_t MIN_FPS{1};
    static constexpr std::uint32_t MAX_FPS{10000};
    static constexpr std::uint32_t DEFAULT_FPS{4000};

    explicit FPSLimiter(std::uint32_t max_fps = DEFAULT_FPS);

    void set_max_fps(std::uint32_t max_fps);

    /// Returns true when enough time has elapsed for the next frame.
    [[nodiscard]] bool is_next_frame_allowed();

    /// Records one rendered frame and returns an updated FPS value periodically.
    [[nodiscard]] std::optional<std::uint32_t> get_fps();

    /// Returns the time elapsed since the last allowed frame, in seconds.
    [[nodiscard]] double elapsed_seconds() const noexcept {
        return m_frame_elapsed.count();
    }

private:
    static constexpr Duration FPS_UPDATE_INTERVAL{1.0};

    std::uint32_t m_max_fps{DEFAULT_FPS};
    Duration m_frame_time{1.0 / static_cast<double>(DEFAULT_FPS)};

    Clock::time_point m_last_frame_time;
    Clock::time_point m_last_fps_update_time;

    Duration m_frame_elapsed{0.0};
    std::uint32_t m_frames{0};
};

} // namespace inexor::vulkan_renderer::tools