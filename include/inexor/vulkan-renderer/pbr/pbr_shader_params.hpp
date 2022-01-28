#pragma once

#include <glm/vec4.hpp>

namespace inexor::vulkan_renderer::pbr {

struct ModelPbrShaderParamsUBO {
    glm::vec4 lightDir = glm::vec4(0.73994f, 0.64279f, 0.19827f, 0.0f);
    float exposure = 4.5f;
    float gamma = 2.2f;
    float prefilteredCubeMipLevels = 0;
    float scaleIBLAmbient = 1.0f;
    float debugViewInputs = 0;
    float debugViewEquation = 0;
};

} // namespace inexor::vulkan_renderer::pbr
