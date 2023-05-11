#pragma once

#include <filesystem>
#include <vector>

namespace inexor::vulkan_renderer::io {

class ByteStream {
protected:
    std::vector<std::uint8_t> m_buffer;

    /// Read from file.
    [[nodiscard]] static std::vector<std::uint8_t> read_file(const std::filesystem::path &path);

public:
    ByteStream() = default;
    explicit ByteStream(std::vector<std::uint8_t> buffer);
    /// Read from file.
    explicit ByteStream(const std::filesystem::path &path);

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const std::vector<std::uint8_t> &buffer() const;
};

class ByteStreamReader {
private:
    const ByteStream &m_stream;
    /// Stream iterator.
    std::vector<std::uint8_t>::const_iterator m_iter;

    void check_end(std::size_t size) const;

public:
    explicit ByteStreamReader(const ByteStream &stream);

    [[nodiscard]] std::size_t remaining() const;
    /// Skip 'size' bytes (std::uint8_t).
    void skip(std::size_t size);

    /// Generic read method.
    template <typename T, typename... Args>
    [[nodiscard]] T read(const Args &...);
};

class ByteStreamWriter : public ByteStream {
public:
    using ByteStream::ByteStream;

    /// Generic write method.
    template <typename T>
    void write(const T &value);
};

} // namespace inexor::vulkan_renderer::io
