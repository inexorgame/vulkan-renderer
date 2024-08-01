#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"

#include <memory>

namespace inexor::vulkan_renderer::rendering::imgui {

// Using declaration
using rendering::render_graph::RenderGraph;

/// A rendering class for ImGui
class ImGuiRenderer {
private:
public:
    ImGuiRenderer(std::weak_ptr<RenderGraph> rendergraph);
    ~ImGuiRenderer() = default;
};

} // namespace inexor::vulkan_renderer::rendering::imgui
