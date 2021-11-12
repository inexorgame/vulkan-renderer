#include "inexor/vulkan-renderer/tools/file.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <fstream>

namespace inexor::vulkan_renderer::tools {

std::string get_file_extension_lowercase(std::string file_name) {

    // Get the file extension
    std::string file_extension = file_name.substr(file_name.find_last_of('.') + 1);

    if (file_extension.empty()) {
        throw std::runtime_error("Error: Could not determine file extension!");
    }

    // Convert every letter to lowercase
    std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return file_extension;
}

File::File(const std::string &file_name) {
    if (!load_file(file_name)) {
        throw std::runtime_error("Error: Could not load file " + file_name + "!");
    }
}

bool File::load_file(const std::string &file_name) {
    assert(!file_name.empty());

    // Open stream at the end of the file to read it's size.
    std::ifstream file_to_load(file_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (file_to_load.is_open()) {
        spdlog::debug("File {} has been opened.", file_name);

        // Read the size of the file.
        const auto file_size = file_to_load.tellg();

        // Preallocate memory for the file buffer.
        m_file_data.resize(m_file_size);

        // Reset the file read position to the beginning of the file.
        file_to_load.seekg(0, std::ios::beg);

        // Read the file data.
        file_to_load.read(m_file_data.data(), file_size);

        m_file_size = static_cast<std::size_t>(file_size);

        // Close the file stream.
        file_to_load.close();

        spdlog::debug("File {} has been closed.", file_name.c_str());

        return true;
    }

    spdlog::error("Could not open shader!");
    return false;
}

} // namespace inexor::vulkan_renderer::tools
