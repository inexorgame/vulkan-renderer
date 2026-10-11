#include "inexor/vulkan-renderer/tools/fps_limiter.hpp"

#include <algorithm>
#include <limits>

namespace inexor::vulkan_renderer::tools {

FPSLimiter::FPSLimiter(const std::uint32_t max_fps)
    : m_last_frame_time(Clock::now()), m_last_fps_update_time(m_last_frame_time) {
    set_max_fps(max_fps);
}

void FPSLimiter::set_max_fps(const std::uint32_t max_fps) {
    m_max_fps = std::clamp(max_fps, MIN_FPS, MAX_FPS);
    m_frame_time = Duration{1.0 / static_cast<double>(m_max_fps)};
}

bool FPSLimiter::is_next_frame_allowed(const bool enforce_limit) {
    const auto current_time = Clock::now();
    m_frame_elapsed = current_time - m_last_frame_time;

    if (enforce_limit && m_frame_elapsed < m_frame_time) {
        return false;
    }

    m_last_frame_time = current_time;
    return true;
}

std::optional<std::uint32_t> FPSLimiter::get_fps() {
    ++m_frames;
    if (m_total_frames != std::numeric_limits<std::uint64_t>::max()) {
        ++m_total_frames;
    }

    const auto current_time = Clock::now();
    if (!m_first_frame_time) {
        m_first_frame_time = current_time;
    }
    const Duration fps_elapsed = current_time - m_last_fps_update_time;

    if (fps_elapsed < FPS_UPDATE_INTERVAL) {
        return std::nullopt;
    }

    const auto fps = static_cast<std::uint32_t>(static_cast<double>(m_frames) / fps_elapsed.count());

    m_frames = 0;
    m_last_fps_update_time = current_time;

    return fps;
}

std::optional<double> FPSLimiter::average_fps() const {
    if (!m_first_frame_time || m_total_frames == 0) {
        return std::nullopt;
    }

    const Duration elapsed = Clock::now() - *m_first_frame_time;
    if (elapsed.count() <= 0.0) {
        return std::nullopt;
    }

    return static_cast<double>(m_total_frames) / elapsed.count();
}

} // namespace inexor::vulkan_renderer::tools