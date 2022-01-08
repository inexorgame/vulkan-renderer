#include "inexor/vulkan-renderer/gltf/cpu_data.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/tools/file.hpp"

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

ModelCpuData::ModelCpuData(const std::string &file_name, const std::string &model_name)
    : m_file_name(file_name), m_model_name(model_name) {

    assert(!file_name.empty());
    assert(!model_name.empty());

    const auto file_extension = tools::get_file_extension_lowercase(file_name);

    bool loading_succeeded = false;

    std::string loading_errors;
    std::string loading_warnings;

    if (file_extension == "gltf") {
        spdlog::trace("Loading ASCII glTF file {}", file_name);
        loading_succeeded = m_loader.LoadASCIIFromFile(&m_model, &loading_errors, &loading_warnings, file_name);
    } else if (file_extension == "glb") {
        spdlog::trace("Loading binary glTF file {}", file_name);
        loading_succeeded = m_loader.LoadBinaryFromFile(&m_model, &loading_errors, &loading_warnings, file_name);
    } else if (file_extension == "obj") {
        throw InexorException("Error: Object files (.obj) are not supported. Use glTF2 format!");
    } else if (file_extension == "fbx") {
        throw InexorException("Error: Autodesk filmbox format (.fbx) is not supported. Use glTF2 format!");
    } else if (file_extension == "blend") {
        throw InexorException("Error: Blender project maps (.blend) are not supported yet. Use glTF2 format!");
    } else {
        throw InexorException("Error: Unknown file extension " + file_extension + "!");
    }

    if (!loading_succeeded) {
        throw InexorException("Error: Failed to load glTF2 file " + file_name + "!");
    }

    if (!loading_warnings.empty()) {
        spdlog::warn(loading_warnings);
    }

    if (!loading_errors.empty()) {
        spdlog::error(loading_errors);
    }
}

ModelCpuData::ModelCpuData(ModelCpuData &&other) noexcept {
    m_file_name = std::move(other.m_file_name);
    m_model_name = std::move(other.m_model_name);
    m_model = std::move(other.m_model);
    m_loader = std::move(other.m_loader);
}

} // namespace inexor::vulkan_renderer::gltf
