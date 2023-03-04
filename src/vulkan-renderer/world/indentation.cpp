#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

namespace inexor::vulkan_renderer::world {
Indentation::Indentation(const std::uint8_t start, const std::uint8_t end) noexcept : m_start(start), m_end(end) {}

Indentation::Indentation(const std::uint8_t uid) noexcept {
    assert(uid <= 44);
    constexpr std::array<std::uint8_t, Indentation::MAX> MASKS{44, 42, 39, 35, 30, 24, 17, 9};
    for (std::uint8_t idx = 0; idx < Indentation::MAX; idx++) {
        if (MASKS[idx] <= uid) {
            m_start = Indentation::MAX - idx;
            m_end = m_start + (uid - MASKS[idx]);
            return;
        }
    }
    m_start = 0;
    m_end = uid;
}

bool Indentation::operator==(const Indentation &rhs) const {
    return m_start == rhs.m_start && m_end == rhs.m_end;
}

bool Indentation::operator!=(const Indentation &rhs) const {
    return !(*this == rhs);
}

void Indentation::set_start(const std::uint8_t position) noexcept {
    m_start = std::clamp<std::uint8_t>(position, 0, Indentation::MAX);
    m_end = std::clamp<std::uint8_t>(m_end, m_start, Indentation::MAX);
}

void Indentation::set_end(const std::uint8_t position) noexcept {
    m_end = std::clamp<std::uint8_t>(position, 0, Indentation::MAX);
    m_start = std::clamp<std::uint8_t>(m_start, 0, m_end);
}

std::uint8_t Indentation::start_abs() const noexcept {
    return m_start;
}

std::uint8_t Indentation::end_abs() const noexcept {
    return m_end;
}

std::uint8_t Indentation::start() const noexcept {
    return m_start;
}

std::uint8_t Indentation::end() const noexcept {
    return Indentation::MAX - m_end;
}

std::uint8_t Indentation::offset() const noexcept {
    return m_end - m_start;
}

void Indentation::indent_start(const std::uint8_t steps) noexcept {
    set_start(m_start + steps);
}

void Indentation::indent_end(const std::uint8_t steps) noexcept {
    set_end(m_end - steps);
}

void Indentation::mirror() noexcept {
    std::uint8_t new_start = end();
    m_end = Indentation::MAX - m_start;
    m_start = new_start;
}

std::uint8_t Indentation::uid() const {
    return static_cast<std::uint8_t>(10 * m_start + offset() - (std::pow(m_start, 2) + m_start) / 2);
}
} // namespace inexor::vulkan_renderer::world
