#pragma once

#include <functional>
// TODO: Move to .cpp!
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

/// A wrapper for graphics stages
class GraphicsStage {
private:
    // A function which is called when the graphics stage is set up
    // TODO: This function should accept a graphics stage builder as parameter and expect a GraphicsStage to be returned
    std::function<void(void)> m_on_stage_setup{[]() {}};

    // TODO: Members which describe graphics stage stuff...

public:
    GraphicsStage() {}

    GraphicsStage(const GraphicsStage &) = delete;

    // TODO: Move to .cpp!
    GraphicsStage(GraphicsStage &&other) noexcept {
        m_on_stage_setup = std::move(other.m_on_stage_setup);
    }
    ~GraphicsStage() = default;

    GraphicsStage &operator=(const GraphicsStage &) = delete;
    GraphicsStage &operator=(GraphicsStage &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
