#pragma once

#include "inexor/vulkan-renderer/gltf-model/node.hpp"

#include <memory>

namespace inexor::vulkan_renderer::gltf_model {

///
struct InexorModelAnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE };

    PathType path;

    std::shared_ptr<InexorModelNode> node;

    uint32_t samplerIndex;
};

} // namespace inexor::vulkan_renderer::gltf_model
