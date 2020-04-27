#pragma once

#include "inexor/vulkan-renderer/gltf-model/node.hpp"

#include <memory>

namespace inexor::vulkan_renderer::gltf_model {

struct AnimationChannel {
    enum class PathType { translation, rotation, scale };

    PathType path;

    std::shared_ptr<ModelNode> node;

    uint32_t samplerIndex;
};

} // namespace inexor::vulkan_renderer::gltf_model
