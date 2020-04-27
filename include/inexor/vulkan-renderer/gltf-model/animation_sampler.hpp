#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

struct AnimationSampler {
    enum class InterpolationType { linear, step, cubic_spline };

    InterpolationType interpolation;

    std::vector<float> inputs;

    std::vector<glm::vec4> outputsVec4;
};

} // namespace inexor::vulkan_renderer::gltf_model
