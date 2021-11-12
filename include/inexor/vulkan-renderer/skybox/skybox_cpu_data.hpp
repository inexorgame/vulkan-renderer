#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <string>

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
    // TODO: Implement!
    SkyboxCpuData(const std::string &file_name, const std::string &model_name);
};

} // namespace inexor::vulkan_renderer::skybox
