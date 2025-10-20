#include "inexor/vulkan-renderer/octree/serialization/byte_stream.hpp"

#include "inexor/vulkan-renderer/octree/cube.hpp"

#include <fstream>

namespace inexor::vulkan_renderer::serialization {

std::vector<std::uint8_t> ByteStream::read_file(const std::filesystem::path &path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

ByteStream::ByteStream(std::vector<std::uint8_t> buffer) : m_buffer(std::move(buffer)) {}

ByteStream::ByteStream(const std::filesystem::path &path) : ByteStream(read_file(path)) {}

const std::vector<std::uint8_t> &ByteStream::buffer() const {
    return m_buffer;
}

std::size_t ByteStream::size() const {
    return m_buffer.size();
}

void ByteStreamReader::check_end(const std::size_t size) const {
    if (static_cast<std::size_t>(std::distance(m_iter, m_stream.buffer().end())) < size) {
        throw std::runtime_error("Error: end of byte stream would be overrun");
    }
}

ByteStreamReader::ByteStreamReader(const ByteStream &stream) : m_stream(stream), m_iter(stream.buffer().begin()) {}

void ByteStreamReader::skip(const std::size_t size) {
    const std::size_t skip = std::min(
        size, std::size_t(std::distance<std::vector<std::uint8_t>::const_iterator>(m_iter, m_stream.buffer().end())));
    std::advance(m_iter, skip);
}

template <>
std::uint8_t ByteStreamReader::read() {
    check_end(1);
    return *m_iter++;
}

template <>
std::uint32_t ByteStreamReader::read() {
    check_end(4);
    return (*m_iter++ << 0u) | (*m_iter++ << 8u) | (*m_iter++ << 16u) | (*m_iter++ << 24u);
}

template <>
std::string ByteStreamReader::read(const std::size_t &size) {
    check_end(size);
    auto start = m_iter;
    std::advance(m_iter, size);
    return {start, m_iter};
}

template <>
octree::Cube::Type ByteStreamReader::read() {
    return static_cast<octree::Cube::Type>(read<std::uint8_t>());
}

template <>
std::array<octree::Indentation, 12> ByteStreamReader::read() {
    check_end(9);
    std::array<octree::Indentation, 12> indentations;
    auto writer = indentations.begin(); // NOLINT
    const auto end = m_iter + 9;
    while (m_iter != end) {
        *writer++ = octree::Indentation(*m_iter >> 2u);
        *writer++ = octree::Indentation(((*m_iter & 0b00000011u) << 4u) | (*(++m_iter) >> 4u));
        *writer++ = octree::Indentation(((*m_iter & 0b00001111u) << 2u) | (*(++m_iter) >> 6u));
        *writer++ = octree::Indentation(*m_iter++ & 0b00111111u);
    }
    return indentations;
}

std::size_t ByteStreamReader::remaining() const {
    return std::distance<std::vector<std::uint8_t>::const_iterator>(m_iter, m_stream.buffer().end());
}

template <>
void ByteStreamWriter::write(const std::uint8_t &value) {
    m_buffer.emplace_back(value);
}

template <>
void ByteStreamWriter::write(const std::uint32_t &value) {
    m_buffer.emplace_back(value >> 24u);
    m_buffer.emplace_back(value >> 16u);
    m_buffer.emplace_back(value >> 8u);
    m_buffer.emplace_back(value);
}

template <>
void ByteStreamWriter::write(const std::string &value) {
    std::copy(value.begin(), value.end(), std::back_inserter(m_buffer));
}

template <>
void ByteStreamWriter::write(const octree::Cube::Type &value) {
    write(static_cast<std::uint8_t>(value));
}

template <>
void ByteStreamWriter::write(const std::array<octree::Indentation, 12> &value) {
    for (auto iter = value.begin(); iter != value.end(); iter++) { // NOLINT
        write<std::uint8_t>((iter->uid() << 2u) | ((++iter)->uid() >> 4));
        write<std::uint8_t>((iter->uid() << 4u) | ((++iter)->uid() >> 2));
        write<std::uint8_t>((iter->uid() << 6u) | ((++iter)->uid()));
    }
}
} // namespace inexor::vulkan_renderer::serialization
