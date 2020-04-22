#pragma once

#include "inexor/vulkan-renderer/gltf-model/animation_channel.hpp"
#include "inexor/vulkan-renderer/gltf-model/animation_sampler.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

///
struct InexorModelAnimation {
    std::string name;

    std::vector<InexorModelAnimationSampler> samplers;

    std::vector<InexorModelAnimationChannel> channels;

    float start = std::numeric_limits<float>::max();

    float end = std::numeric_limits<float>::min();
};

} // namespace inexor::vulkan_renderer::gltf_model
