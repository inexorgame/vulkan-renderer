#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::tools {

/// @brief A class for loading files into memory.
/// @todo Refactor into an RAII wrapper.
class File {
private:
    /// The file data.
    std::vector<char> m_file_data;

    /// The size of the file.
    std::size_t m_file_size{0};

public:
    File() = default;

    [[nodiscard]] std::size_t file_size() const {
        return m_file_size;
    }

    [[nodiscard]] const std::vector<char> &file_data() const {
        return m_file_data;
    }

    /// @brief Load the entire file into memory.
    /// @param file_name The name of the file.
    /// @return ``true`` if file was loaded successfully.
    [[nodiscard]] bool load_file(const std::string &file_name);
};

} // namespace inexor::vulkan_renderer::tools
