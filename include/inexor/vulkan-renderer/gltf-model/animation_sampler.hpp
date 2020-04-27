#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

struct AnimationSampler {
    enum class InterpolationType { LINEAR, STEP, CUBIC_SPLINE };

    InterpolationType interpolation;

    std::vector<float> inputs;

    std::vector<glm::vec4> outputs_vec4;
};

} // namespace inexor::vulkan_renderer::gltf_model
