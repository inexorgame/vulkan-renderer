#pragma once

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include <string>

namespace inexor::vulkan_renderer::gltf2 {

/// @brief A wrapper class for glTF2 files based on tinygltf library.
/// The loading of a glTF2 file is kept separate from its rendering setup to allow for easier splitting of tasks later
/// on. This will allow for better parallelization.
class glTF2_File {
private:
    tinygltf::Model m_model;
    tinygltf::TinyGLTF m_loader;

public:
    /// @brief Load a glTF2 file of a given filename.
    /// @param file_name The name of the glTF2 file.
    /// @note The constructor automatically checks for the file extension and loads the glTF2 file either
    /// as ASCII (.gltf) or binary file (.glb).
    glTF2_File(const std::string &file_name);

    glTF2_File(const glTF2_File &) = delete;
    glTF2_File(glTF2_File &&) = delete;

    ~glTF2_File();

    glTF2_File &operator=(const glTF2_File &) = delete;
    glTF2_File &operator=(glTF2_File &&) = delete;
};

} // namespace inexor::vulkan_renderer::gltf2
