#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string>

namespace inexor::vulkan_renderer::gltf {

/// @brief A wrapper class for glTF2 model files based on tinygltf library
/// https://github.com/syoyo/tinygltf
/// The loading of a glTF2 file is kept separate from its rendering setup to allow for easier splitting of tasks later
/// on. This will allow for better parallelization
class ModelCpuData {
private:
    std::string m_file_name;
    std::string m_model_name;

    tinygltf::Model m_model;
    tinygltf::TinyGLTF m_loader;

public:
    ///
    ///
    ///
    ModelCpuData(const std::string &file_name, const std::string &model_name);

    ModelCpuData(const ModelCpuData &) = delete;
    ModelCpuData(ModelCpuData &&) noexcept;

    ModelCpuData &operator=(const ModelCpuData &) = delete;
    ModelCpuData &operator=(ModelCpuData &&) = default;

    [[nodiscard]] const tinygltf::Model &model() const {
        return m_model;
    }

    [[nodiscard]] const std::string &file_name() const {
        return m_file_name;
    }

    [[nodiscard]] const std::string &model_name() const {
        return m_model_name;
    }
};

} // namespace inexor::vulkan_renderer::gltf
