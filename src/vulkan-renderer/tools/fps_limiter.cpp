#include "inexor/vulkan-renderer/tools/fps_limiter.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::tools {

FPSLimiter::FPSLimiter(const std::uint32_t max_fps)
    : m_last_frame_time(Clock::now()), m_last_fps_update_time(m_last_frame_time) {
    set_max_fps(max_fps);
}

void FPSLimiter::set_max_fps(const std::uint32_t max_fps) {
    m_max_fps = std::clamp(max_fps, MIN_FPS, MAX_FPS);
    m_frame_time = Duration{1.0 / static_cast<double>(m_max_fps)};
}

bool FPSLimiter::is_next_frame_allowed() {
    const auto current_time = Clock::now();
    m_frame_elapsed = current_time - m_last_frame_time;

    if (m_frame_elapsed < m_frame_time) {
        return false;
    }

    m_last_frame_time = current_time;
    return true;
}

std::optional<std::uint32_t> FPSLimiter::get_fps() {
    ++m_frames;

    const auto current_time = Clock::now();
    const Duration fps_elapsed = current_time - m_last_fps_update_time;

    if (fps_elapsed < FPS_UPDATE_INTERVAL) {
        return std::nullopt;
    }

    const auto fps = static_cast<std::uint32_t>(static_cast<double>(m_frames) / fps_elapsed.count());

    m_frames = 0;
    m_last_fps_update_time = current_time;

    return fps;
}

} // namespace inexor::vulkan_renderer::tools