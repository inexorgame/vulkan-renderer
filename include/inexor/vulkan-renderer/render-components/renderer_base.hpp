#pragma once

#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <memory>

namespace inexor::vulkan_renderer::render_components {

// Using declaration
using render_graph::GraphicsPass;

/// An abstract base class for renderers
class RendererBase {
protected:
    std::weak_ptr<GraphicsPass> m_graphics_pass;

public:
    /// We need this public get method because other renderers might need to know about the previous graphics pass.
    std::weak_ptr<GraphicsPass> get_pass() const {
        return m_graphics_pass;
    }
};

} // namespace inexor::vulkan_renderer::render_components
