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

    ~File() = default;

    /// @brief Return the size of the file.
    [[nodiscard]] const std::size_t file_size() const;

    /// @brief Return the file's data.
    [[nodiscard]] const std::vector<char> &file_data() const;

    /// @brief Load the entire file into memory.
    /// @param file_name The name of the file.
    /// @return ``true`` if file was loaded successfully.
    [[nodiscard]] bool load_file(const std::string &file_name);
};

} // namespace inexor::vulkan_renderer::tools
