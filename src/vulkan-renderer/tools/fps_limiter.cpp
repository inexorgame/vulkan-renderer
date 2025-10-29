#include "inexor/vulkan-renderer/tools/fps_limiter.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::tools {

FPSLimiter::FPSLimiter(const std::uint32_t max_fps) {
    set_max_fps(max_fps);
}

void FPSLimiter::set_max_fps(std::uint32_t max_fps) {
    m_max_fps = std::clamp(max_fps, MIN_FPS, MAX_FPS);
    m_frame_time = std::chrono::milliseconds(1000) / m_max_fps;
}

bool FPSLimiter::is_next_frame_allowed() {
    const auto current_time = std::chrono::high_resolution_clock::now();
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_last_time);
    if (elapsed_ms >= m_frame_time) {
        m_last_time = current_time;
        return true;
    }
    return false;
}

std::optional<std::uint32_t> FPSLimiter::get_fps() {
    m_frames++;
    const auto current_time = std::chrono::high_resolution_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_last_fps_update_time);
    if (elapsed_ms >= m_fps_update_interval) {
        m_last_fps_update_time = current_time;
        auto fps_value = static_cast<std::uint32_t>(m_frames * 1000.0f / elapsed_ms.count());
        m_frames = 0;
        return fps_value;
    }

    return std::nullopt;
}

} // namespace inexor::vulkan_renderer::tools
