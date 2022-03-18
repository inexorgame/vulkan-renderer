#include "inexor/vulkan-renderer/tools/file.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>

namespace inexor::vulkan_renderer::tools {

std::string get_file_extension_lowercase(const std::string &file_name) {
    assert(!file_name.empty());

    // Extract the file extension
    std::string file_extension = std::filesystem::path(file_name).extension().string();

    if (file_extension.empty()) {
        return "";
    }

    // Convert file extension string to lowercase
    std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return file_extension;
}

std::vector<char> read_file_binary_data(const std::string &file_name) {

    // Open stream at the end of the file to read it's size.
    std::ifstream file(file_name.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

    if (!file) {
        throw std::runtime_error("Error: Could not open file " + file_name + "!");
    }

    // Read the size of the file
    const auto file_size = file.tellg();

    std::vector<char> buffer(file_size);

    // Set the file read position to the beginning of the file
    file.seekg(0);

    file.read(buffer.data(), file_size);

    return buffer;
}

} // namespace inexor::vulkan_renderer::tools
