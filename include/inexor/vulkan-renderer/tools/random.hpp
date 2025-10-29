#pragma once

#include <concepts>
#include <optional>
#include <random>
#include <type_traits>

namespace inexor::vulkan_renderer::tools {

/// Generates a random number of arithmetic type T in between the bounds `min` and `max`.
inline auto generate_random_number =
    []<typename T>(const T min, const T max, const std::optional<std::uint32_t> seed = std::nullopt)
    requires std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>
{
    // Note that thread_local means that it is implicitely static!
    thread_local std::mt19937 generator(seed.value_or(std::random_device{}()));
    using U = std::decay_t<T>;
    if constexpr (std::is_integral_v<U>) {
        std::uniform_int_distribution<U> distribution(min, max);
        return distribution(generator);
    } else if constexpr (std::is_floating_point_v<U>) {
        std::uniform_real_distribution<U> distribution(min, max);
        return distribution(generator);
    } else {
        static_assert(std::is_arithmetic_v<U>, "Error: Type must be numeric (integer or float)!");
    }
};

} // namespace inexor::vulkan_renderer::tools
