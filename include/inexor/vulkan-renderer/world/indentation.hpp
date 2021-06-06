#pragma once

#include <cstdint>

namespace inexor::vulkan_renderer::world {

class Indentation {
public:
    static constexpr std::uint8_t MAX{8};

private:
    std::uint8_t m_start{0};
    std::uint8_t m_end{Indentation::MAX};

public:
    Indentation() = default;
    Indentation(std::uint8_t start, std::uint8_t end) noexcept;
    explicit Indentation(std::uint8_t uid) noexcept;
    bool operator==(const Indentation &rhs) const;
    bool operator!=(const Indentation &rhs) const;

    /// Set absolute value of start.
    void set_start(std::uint8_t position) noexcept;
    /// Set absolute value of end.
    void set_end(std::uint8_t position) noexcept;

    /// Absolute value of start.
    [[nodiscard]] std::uint8_t start_abs() const noexcept;
    /// Absolute value of end.
    [[nodiscard]] std::uint8_t end_abs() const noexcept;
    /// Positive indent, relative from start's point.
    [[nodiscard]] std::uint8_t start() const noexcept;
    /// Positive indent, relative from end's point.
    [[nodiscard]] std::uint8_t end() const noexcept;
    /// Difference between start and end.
    [[nodiscard]] std::uint8_t offset() const noexcept;

    /// Positive steps into the direction of end.
    void indent_start(std::uint8_t steps) noexcept;
    /// Positive steps into the direction of start.
    void indent_end(std::uint8_t steps) noexcept;
    /// Mirror the indentation, such that the distance from 0 to start and distance from end to max switches
    void mirror() noexcept;

    [[nodiscard]] std::uint8_t uid() const;
};

} // namespace inexor::vulkan_renderer::world
