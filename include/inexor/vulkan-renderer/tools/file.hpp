#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::tools {

/// @brief A class for loading files into memory.
class File {
private:
    /// The file data.
    std::vector<char> file_data;

    /// The size of the file.
    std::size_t file_size;

public:
    File() = default;

    ~File() = default;

    /// @brief Returns the size of the file.
    [[nodiscard]] const std::size_t get_file_size() const;

    /// @brief Returns the file's data.
    [[nodiscard]] const std::vector<char> &get_file_data() const;

    /// @brief Loads the entire file into memory.
    /// @param file_name The name of the file.
    /// @return True if file was loaded successfully, false otherwise.
    [[nodiscard]] bool load_file(const std::string &file_name);
};

} // namespace inexor::vulkan_renderer::tools
