#pragma once

// @TODO Forward-declare as much as possible!
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-modules/imgui/imgui_renderer.hpp"
#include "inexor/vulkan-renderer/render-modules/octree/octree_renderer.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/fps_limiter.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/tools/time_step.hpp"
#include "inexor/vulkan-renderer/wrapper/core/debug_callback.hpp"
#include "inexor/vulkan-renderer/wrapper/core/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declarations
class Device;
class Instance;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer::tools {
// Forward declarations
class Camera;
class FPSLimiter;
class TimeStep;
} // namespace inexor::vulkan_renderer::tools

namespace inexor::vulkan_renderer::render_modules::imgui {
// Forward declaration
class ImGuiRenderer;
} // namespace inexor::vulkan_renderer::render_modules::imgui

namespace inexor::vulkan_renderer::render_modules::imgui {
// Forward declaration
class OctreeRenderer;
} // namespace inexor::vulkan_renderer::render_modules::imgui

namespace inexor::vulkan_renderer::wrapper::windows {
// Forward declaration
class Window;
class WindowSurface;
} // namespace inexor::vulkan_renderer::wrapper::windows

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::example_app {

// Using declarations
using vulkan_renderer::render_graph::RenderGraph;
using vulkan_renderer::render_graph::Texture;
using vulkan_renderer::render_modules::imgui::ImGuiRenderer;
using vulkan_renderer::render_modules::octree::OctreeRenderer;
using vulkan_renderer::tools::Camera;
using vulkan_renderer::tools::FPSLimiter;
using vulkan_renderer::wrapper::core::Device;
using vulkan_renderer::wrapper::core::Instance;
using vulkan_renderer::wrapper::core::VulkanDebugUtilsCallback;
using vulkan_renderer::wrapper::swapchains::Swapchain;
using vulkan_renderer::wrapper::windows::Window;
using vulkan_renderer::wrapper::windows::WindowSurface;

/// The base class of the Inexor vulkan-renderer example app.
class ExampleAppBase {
protected:
    /// NOTE: The declaration order of members determines the order of destruction!
    /// This means we must keep the instance before the debug utils messenger callback!
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<VulkanDebugUtilsCallback> m_dbg_callback;

    std::unique_ptr<Device> m_device;

    std::unique_ptr<WindowSurface> m_surface;
    std::shared_ptr<Swapchain> m_swapchain;
    std::shared_ptr<RenderGraph> m_render_graph;
    std::weak_ptr<Texture> m_depth_buffer;

    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<Window> m_window;

    std::unique_ptr<ImGuiRenderer> m_imgui_renderer;
    std::unique_ptr<OctreeRenderer> m_octree_renderer;

    bool m_vsync_enabled{false};

    // @TODO Move to window wrapper!
    bool m_window_resized{false};

    FPSLimiter m_fps_limiter;

public:
    ExampleAppBase();
    ~ExampleAppBase();
};

} // namespace inexor::example_app
