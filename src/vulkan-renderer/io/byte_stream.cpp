#include "inexor/vulkan-renderer/io/byte_stream.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <fstream>

namespace inexor::vulkan_renderer::io {
std::vector<std::uint8_t> ByteStream::read_file(const std::filesystem::path &path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

ByteStream::ByteStream(std::vector<std::uint8_t> buffer) : m_buffer(std::move(buffer)) {}

ByteStream::ByteStream(const std::filesystem::path &path) : ByteStream(read_file(path)) {}

std::size_t ByteStream::size() const {
    return m_buffer.size();
}
const std::vector<std::uint8_t> &ByteStream::buffer() const {
    return m_buffer;
}

void ByteStreamReader::check_end(const std::size_t size) const {
    if (static_cast<std::size_t>(std::distance(m_iter, m_stream.buffer().end())) < size) {
        throw std::runtime_error("end would be overrun");
    }
}

ByteStreamReader::ByteStreamReader(const ByteStream &stream) : m_stream(stream), m_iter(stream.buffer().begin()) {}

void ByteStreamReader::skip(const std::size_t size) {
    const std::size_t skip = std::min(
        size, std::size_t(std::distance<std::vector<std::uint8_t>::const_iterator>(m_iter, m_stream.buffer().end())));
    std::advance(m_iter, skip);
}

std::size_t ByteStreamReader::remaining() const {
    return std::distance<std::vector<std::uint8_t>::const_iterator>(m_iter, m_stream.buffer().end());
}

template <>
std::uint8_t ByteStreamReader::read() {
    check_end(1);
    return *m_iter++;
}

template <>
std::uint32_t ByteStreamReader::read() {
    check_end(4);
    return (*m_iter++ << 0U) | (*m_iter++ << 8U) | (*m_iter++ << 16U) | (*m_iter++ << 24U);
}

template <>
std::string ByteStreamReader::read(const std::size_t &size) {
    check_end(size);
    auto start = m_iter;
    std::advance(m_iter, size);
    return std::string(start, m_iter);
}

template <>
world::Cube::Type ByteStreamReader::read() {
    return static_cast<world::Cube::Type>(read<std::uint8_t>());
}

template <>
std::array<world::Indentation, 12> ByteStreamReader::read() {
    check_end(9);
    std::array<world::Indentation, 12> indentations;
    auto writer = indentations.begin();
    const auto end = m_iter + 9;
    while (m_iter != end) {
        *writer++ = world::Indentation(*m_iter >> 2U);
        *writer++ = world::Indentation(((*m_iter & 0b00000011U) << 4U) | (*(++m_iter) >> 4U));
        *writer++ = world::Indentation(((*m_iter & 0b00001111U) << 2U) | (*(++m_iter) >> 6U));
        *writer++ = world::Indentation(*m_iter++ & 0b00111111U);
    }
    return indentations;
}

template <>
void ByteStreamWriter::write(const std::uint8_t &value) {
    m_buffer.emplace_back(value);
}

template <>
void ByteStreamWriter::write(const std::uint32_t &value) {
    m_buffer.emplace_back(value >> 24U);
    m_buffer.emplace_back(value >> 16U);
    m_buffer.emplace_back(value >> 8U);
    m_buffer.emplace_back(value);
}

template <>
void ByteStreamWriter::write(const std::string &value) {
    std::copy(value.begin(), value.end(), std::back_inserter(m_buffer));
}

template <>
void ByteStreamWriter::write(const world::Cube::Type &value) {
    write(static_cast<std::uint8_t>(value));
}

template <>
void ByteStreamWriter::write(const std::array<world::Indentation, 12> &value) {
    for (auto iter = value.begin(); iter != value.end(); iter++) {
        write<std::uint8_t>((iter->uid() << 2U) | ((++iter)->uid() >> 4));
        write<std::uint8_t>((iter->uid() << 4U) | ((++iter)->uid() >> 2));
        write<std::uint8_t>((iter->uid() << 6U) | ((++iter)->uid()));
    }
}
} // namespace inexor::vulkan_renderer::io
