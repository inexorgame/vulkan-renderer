#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_node.hpp"

#include <glm/vec4.hpp>
#include <limits>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

struct ModelAnimationChannel {
    enum class PathType { TRANSLATION, ROTATION, SCALE };
    PathType path;
    ModelNode *node;
    std::uint32_t sampler_index;
};

struct ModelAnimationSampler {
    enum class InterpolationType { LINEAR, STEP, CUBICSPLINE };
    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputs;
};

struct ModelSkin {
    std::string name;
    ModelNode *skeleton_root{nullptr};
    std::vector<glm::mat4> inverse_bind_matrices;
    std::vector<ModelNode *> joints;
};

struct ModelAnimation {
    std::string name;
    std::vector<ModelAnimationSampler> samplers;
    std::vector<ModelAnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};

} // namespace inexor::vulkan_renderer::gltf
