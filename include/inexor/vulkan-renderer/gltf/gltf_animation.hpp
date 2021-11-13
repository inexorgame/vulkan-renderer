#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <limits>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

struct ModelNode;

enum class AnimationPathType { TRANSLATION, ROTATION, SCALE };

enum class AnimationInterpolationType { LINEAR, STEP, CUBICSPLINE };

struct ModelAnimationChannel {
    AnimationPathType path;
    ModelNode *node{nullptr};
    std::uint32_t sampler_index;
};

struct ModelAnimationSampler {
    AnimationInterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputs;
};

struct ModelAnimation {
    std::string name;
    std::vector<ModelAnimationSampler> samplers;
    std::vector<ModelAnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};

} // namespace inexor::vulkan_renderer::gltf
