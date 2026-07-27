#include "renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/windows/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/window.hpp"

namespace inexor::example_app {

ExampleAppBase::ExampleAppBase() {}

ExampleAppBase::~ExampleAppBase() {
    spdlog::trace("Shutting down vulkan renderer");
    m_device->wait_idle();
    spdlog::trace("Releasing octree renderer");
    m_octree_renderer.reset();
    spdlog::trace("Releasing imgui renderer");
    m_imgui_renderer.reset();
    spdlog::trace("Resetting render graph");
    if (m_render_graph) {
        m_render_graph->reset_graph();
    }
    spdlog::trace("Releasing render graph");
    m_render_graph.reset();
    spdlog::trace("Releasing swapchain");
    m_swapchain.reset();
    spdlog::trace("Releasing surface");
    m_surface.reset();
    spdlog::trace("Releasing window");
    m_window.reset();
    spdlog::trace("Releasing camera");
    m_camera.reset();
    spdlog::trace("Releasing device");
    m_device.reset();
    spdlog::trace("Releasing debug callback");
    m_dbg_callback.reset();
    spdlog::trace("Releasing instance");
    m_instance.reset();
}

} // namespace inexor::example_app
