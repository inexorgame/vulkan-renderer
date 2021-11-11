#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_file.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer::skybox {

class SkyboxCpuData {
private:
    // TODO: Put this into its own wrapper file
    struct UBOMatrices {
        glm::mat4 projection;
        glm::mat4 model;
        glm::mat4 view;
        glm::vec3 camPos;
    };

    UBOMatrices m_skybox_data;

public:
    SkyboxCpuData(const gltf::ModelFile &skybox_model);
};

} // namespace inexor::vulkan_renderer::skybox
