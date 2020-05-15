#pragma once

#include <boost/dynamic_bitset.hpp>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer::world {
constexpr std::array<std::uint8_t, 9> KEEP_FIRST_N_BITS{0b0000'0000, 0b1000'0000, 0b1100'0000, 0b1110'0000, 0b1111'0000,
                                                        0b1111'1000, 0b1111'1100, 0b1111'1110, 0b1111'1111};

constexpr std::array<std::uint8_t, 9> DISCARD_FIRST_N_BITS{0b1111'1111, 0b0111'1111, 0b0011'1111, 0b0001'1111, 0b0000'1111,
                                                           0b0000'0111, 0b0000'0011, 0b0000'0001, 0b0000'0000};

/// Create a BitStream
/// Extract a certain number of bits from binary data e.g. for binary file parsing.
class BitStream {
private:
    /// Pointer to the current data.
    unsigned char *data{};

    /// Offset in the current byte from the start of the byte.
    std::uint8_t offset{};

    /// Number of bytes left in the stream.
    std::size_t bytes_left{};

public:
    /// The data to create bitstream from.
    /// @param data
    /// @param byte_size
    explicit BitStream(unsigned char *data, std::size_t byte_size);

    BitStream();

    /// Get size bits from the stream.
    /// As octree file format only requires up to one-byte values, the function is restricted to that size (for easier implementation).
    /// @param size Bits to get (<9).
    /// @return <size> next bits of the stream.
    std::optional<std::uint8_t> get(std::uint8_t size);

    /// Get size bits from the stream.
    /// As octree file format only requires up to one-byte values, the function is restricted to that size (for easier implementation).
    /// @param size Bits to get (<9).
    /// @return <size> next bits of the stream.
    std::optional<boost::dynamic_bitset<>> get_bitset(std::uint8_t size);
};
} // namespace inexor::vulkan_renderer::world
