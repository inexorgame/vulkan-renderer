#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string>

namespace inexor::vulkan_renderer::gltf {

/// @brief A wrapper class for glTF2 model files based on tinygltf library.
/// https://github.com/syoyo/tinygltf
/// The loading of a glTF2 file is kept separate from its rendering setup to allow for easier splitting of tasks later
/// on. This will allow for better parallelization.
class ModelFile {
private:
    tinygltf::Model m_model;
    tinygltf::TinyGLTF m_loader;
    std::string m_file_name;
    std::string m_model_name;

public:
    /// @brief Load a glTF2 model file.
    /// @note The constructor automatically checks for the file extension and loads the glTF2 file either as ASCII
    /// (.gltf) or binary file (.glb).
    /// @param file_name The name of the glTF2 file
    /// @param m_model_name The internal model name of the model which should be user friendly to be displayed to the
    /// user
    ModelFile(const std::string &file_name, const std::string &model_name);

    [[nodiscard]] const tinygltf::Model &model() const noexcept {
        return m_model;
    }

    [[nodiscard]] const std::string &file_name() const noexcept {
        return m_file_name;
    }

    [[nodiscard]] const std::string &model_name() const noexcept {
        return m_model_name;
    }
};

} // namespace inexor::vulkan_renderer::gltf
