#pragma once

namespace inexor::vulkan_renderer::render_graph {

///
class GraphicsStageBuilder {
private:
    //

public:
    GraphicsStageBuilder();
    ~GraphicsStageBuilder() = default;

    ///
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph