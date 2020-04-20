#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace inexor {
namespace vulkan_renderer {

///
struct InexorModelAnimationSampler {
    enum InterpolationType { LINEAR, STEP, CUBICSPLINE };

    InterpolationType interpolation;

    std::vector<float> inputs;

    std::vector<glm::vec4> outputsVec4;
};

}; // namespace vulkan_renderer
}; // namespace inexor
