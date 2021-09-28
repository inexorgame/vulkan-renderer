#include "inexor/vulkan-renderer/gltf/gltf_file.hpp"

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "inexor/vulkan-renderer/exception.hpp"
#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

ModelFile::ModelFile(const std::string &file_name, const std::string &model_name)
    : m_file_name(file_name), m_model_name(model_name) {
    assert(!file_name.empty());
    assert(!model_name.empty());

    // Get the file extension from the file name.
    const std::string file_extension = file_name.substr(file_name.find_last_of('.') + 1);

    if (file_extension.empty()) {
        throw InexorException("Error: Could not determine file extension from " + file_name);
    }

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

    if (!loading_warnings.empty()) {
        spdlog::warn(loading_warnings);
    }

    if (!loading_errors.empty()) {
        spdlog::error(loading_errors);
    }

    if (!loading_succeeded) {
        throw InexorException("Error: Failed to load glTF2 file " + file_name + "!");
    }
}

} // namespace inexor::vulkan_renderer::gltf
