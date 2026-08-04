#include "renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/windows/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/window.hpp"

namespace inexor::example_app {

ExampleAppBase::ExampleAppBase() {}

ExampleAppBase::~ExampleAppBase() {
    // Destroy the render graph first so it can wait for in-flight work before
    // renderer-owned pipeline objects are released.
    m_render_graph.reset();
    m_octree_renderer.reset();
    m_imgui_renderer.reset();
    m_swapchain.reset();
    m_surface.reset();
    m_window.reset();
}

} // namespace inexor::example_app
