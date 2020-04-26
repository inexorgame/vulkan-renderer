#include <inexor/vulkan-renderer/world/BitStream.hpp>

using namespace inexor;

const static uint8_t KEEP_FIRST_N_BITS[9] {0b0000'0000, 0b1000'0000, 0b1100'0000, 0b1110'0000,
                                           0b1111'0000, 0b1111'1000, 0b1111'1100, 0b1111'1110, 0b1111'1111};

const static uint8_t DISCARD_FIRST_N_BITS[9] {0b1111'1111, 0b0111'1111, 0b0011'1111, 0b0001'1111,
                                              0b0000'1111, 0b0000'0111, 0b0000'0011, 0b0000'0001, 0b0000'0000};

BitStream::BitStream(unsigned char *data, size_t size) {
    this->data = data;
    this->offset = 0;
    this->bytes_left = size;
}

BitStream::BitStream() {}

optional<boost::dynamic_bitset<>> BitStream::get_bitset(uint8_t size) {
    const optional<uint8_t> ubits = this->get(size);
    if(ubits == nullopt) {
        return nullopt;
    }
    return boost::dynamic_bitset<>(size, ubits.value());
}

optional<uint8_t> BitStream::get(uint8_t size) {
    // Inexor Octree does not use any data types larger than 8 bits.
    assert(size && size < 9);
    cout << "Bitstream::get - " << (unsigned int) size << " bits." << endl;

    if (!this->bytes_left) {
        cout << "No bytes left!" << endl;
        return nullopt;
    }

    auto current = static_cast<uint8_t>(*this->data);
    if(!this->offset) {
        this->offset = size % 8;
        if(size == 8) {
            this->bytes_left -= 1;
            this->data++;
        }
        return (current & KEEP_FIRST_N_BITS[size]) >> (8 - size);
    }

    uint8_t overflow = this->offset + size <= 8 ? 0 : (size + this->offset) % 8;
    current &= DISCARD_FIRST_N_BITS[this->offset];

    if(!overflow) {
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

    if (!this->bytes_left) {
        cout << "NO BYTES LEFT WHILE REQUIRED!" << endl;
        return nullopt;
    }
    return current | this->get(overflow).value();
}
