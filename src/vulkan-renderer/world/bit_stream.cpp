#include <inexor/vulkan-renderer/world/bit_stream.hpp>

namespace inexor::vulkan_renderer::world {
BitStream::BitStream(unsigned char *data, std::size_t size) {
    this->data = data;
    this->offset = 0;
    this->bytes_left = size;
}

BitStream::BitStream() {}

std::optional<boost::dynamic_bitset<>> BitStream::get_bitset(uint8_t size) {
    const std::optional<uint8_t> ubits = this->get(size);
    if (ubits == std::nullopt) {
        return std::nullopt;
    }
    return boost::dynamic_bitset<>(size, ubits.value());
}

std::optional<uint8_t> BitStream::get(uint8_t size) {
    // Inexor Octree does not use any data types larger than 8 bits.
    assert(size && size < 9);
    assert(this->bytes_left);

    auto current = static_cast<uint8_t>(*this->data);
    if (!this->offset) {
        this->offset = size % 8;
        if (size == 8) {
            this->bytes_left -= 1;
            this->data++;
        }
        return (current & KEEP_FIRST_N_BITS[size]) >> (8 - size);
    }

    uint8_t overflow = this->offset + size <= 8 ? 0 : (size + this->offset) % 8;
    current &= DISCARD_FIRST_N_BITS[this->offset];

    if (!overflow) {
        uint8_t bits = current >> (8 - this->offset - size);
        if ((this->offset + size) == 8) {
            this->offset = 0;
            this->bytes_left -= 1;
            this->data++;
        } else {
            this->offset += size;
        }
        return bits;
    }

    this->offset = 0;
    this->bytes_left -= 1;
    this->data++;

    assert(this->bytes_left);
    return current | this->get(overflow).value();
}
} // namespace inexor::vulkan_renderer::world
