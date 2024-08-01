#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"

#include <memory>

namespace inexor::vulkan_renderer::rendering::octree {

/// A rendering class for octrees
class OctreeRenderer {
private:
public:
    OctreeRenderer(std::weak_ptr<RenderGraph> rendergraph);
    ~OctreeRenderer() = default;
};

} // namespace inexor::vulkan_renderer::rendering::octree
