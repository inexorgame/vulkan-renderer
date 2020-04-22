#pragma once

#include "inexor/vulkan-renderer/gltf-model/node.hpp"

#include <memory>

namespace inexor::vulkan_renderer::gltf_model {

///
struct AnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE };

    PathType path;

    std::shared_ptr<ModelNode> node;

    uint32_t samplerIndex;
};

} // namespace inexor::vulkan_renderer::gltf_model
