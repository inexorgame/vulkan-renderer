#pragma once

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::tools {

/// @brief Extract the extension of a file as lowercase string.
/// @param file_name the name of the file. This is allowed to include the relative or complete path
/// @return The extension of the file as lowercase
[[nodiscard]] std::string get_file_extension_lowercase(const std::string &file_name);

/// @brief Read the data of a file into memory
/// @param file_name The name of the file
/// @return A std::vector of type char which contains the binary data of the file
[[nodiscard]] std::vector<char> read_file_binary_data(const std::string &file_name);

} // namespace inexor::vulkan_renderer::tools
