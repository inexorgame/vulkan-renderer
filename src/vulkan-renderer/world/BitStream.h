//
// Created by moritz on 21.04.20.
//

#ifndef INEXOR_VULKAN_RENDERER_BITSTREAM_H
#define INEXOR_VULKAN_RENDERER_BITSTREAM_H

#include <cstdint>
#include <GL/glew.h>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <cstdint>
#include <iostream>
#include <optional>
#include <boost/dynamic_bitset.hpp>

using namespace std;

namespace inexor {
class BitStream {
    /**
     * Create a BitStream
     * Extract a certain number of bits from binary data e.g. for binary file parsing.
     */
public:
    /**
     * The data to create bitstream from.
     * @param data
     * @param byte_size
     */
    explicit BitStream(unsigned char *data, size_t byte_size);

    BitStream();

    /**
     * Get size bits from the stream.
     * As octree file format only requires up to one-byte values, the function is restricted to that size (for easier implementation).
     * @param size Bits to get (<9).
     * @return <size> next bits of the stream.
     */
    optional<uint8_t> get(uint8_t size);

    /**
     * Get size bits from the stream.
     * As octree file format only requires up to one-byte values, the function is restricted to that size (for easier implementation).
     * @param size Bits to get (<9).
     * @return <size> next bits of the stream.
     */
    optional<boost::dynamic_bitset<>> get_bitset(uint8_t size);

private:
    /**
     * Pointer to the current data.
     */
    unsigned char *data;

    /**
     * Offset in the current byte from the start of the byte.
     */
    uint8_t offset;

    /**
     * Number of bytes left in the stream.
     */
    size_t bytes_left;
};
}

#endif // INEXOR_VULKAN_RENDERER_BITSTREAM_H