#pragma once

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include <string>

namespace inexor::vulkan_renderer::gltf2 {

/// @brief A wrapper class for glTF2 files based on tinygltf library.
/// The loading of a glTF2 file is kept separate from its rendering setup to allow for easier splitting of tasks later
/// on. This will allow for better parallelization.
class gltf2_file {
private:
    tinygltf::Model m_model;
    tinygltf::TinyGLTF m_loader;

public:
    /// @brief Load a glTF2 file of a given filename.
    /// @param file_name The name of the glTF2 file.
    /// @note The constructor automatically checks for the file extension and loads the glTF2 file either
    /// as ASCII (.gltf) or binary file (.glb).
    explicit gltf2_file(const std::string &file_name);

    gltf2_file(const gltf2_file &) = delete;
    gltf2_file(gltf2_file &&) = delete;

    ~gltf2_file();

    gltf2_file &operator=(const gltf2_file &) = delete;
    gltf2_file &operator=(gltf2_file &&) = delete;
};

} // namespace inexor::vulkan_renderer::gltf2
