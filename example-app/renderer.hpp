#pragma once

#include "inexor/vulkan-renderer/imgui.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/fps_limiter.hpp"
#include "inexor/vulkan-renderer/tools/time_step.hpp"
#include "inexor/vulkan-renderer/wrapper/debug_callback.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/window.hpp"
#include "octree_gpu_vertex.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Device;
class GpuTexture;
class Instance;
class Shader;
class UniformBuffer;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class ResourceDescriptor;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

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

namespace inexor::vulkan_renderer {
// Forward declarations
class ImGUIOverlay;
class RenderGraph;
} // namespace inexor::vulkan_renderer

// Using declarations
using inexor::vulkan_renderer::BufferResource;
using inexor::vulkan_renderer::BufferUsage;
using inexor::vulkan_renderer::GraphicsStage;
using inexor::vulkan_renderer::ImGUIOverlay;
using inexor::vulkan_renderer::PhysicalStage;
using inexor::vulkan_renderer::RenderGraph;
using inexor::vulkan_renderer::TextureResource;
using inexor::vulkan_renderer::TextureUsage;
using inexor::vulkan_renderer::tools::Camera;
using inexor::vulkan_renderer::tools::CameraMovement;
using inexor::vulkan_renderer::tools::CameraType;
using inexor::vulkan_renderer::tools::FPSLimiter;
using inexor::vulkan_renderer::tools::TimeStep;
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::GpuTexture;
using inexor::vulkan_renderer::wrapper::Instance;
using inexor::vulkan_renderer::wrapper::Shader;
using inexor::vulkan_renderer::wrapper::UniformBuffer;
using inexor::vulkan_renderer::wrapper::VulkanDebugUtilsCallback;
using inexor::vulkan_renderer::wrapper::commands::CommandBuffer;
using inexor::vulkan_renderer::wrapper::descriptors::DescriptorBuilder;
using inexor::vulkan_renderer::wrapper::descriptors::ResourceDescriptor;
using inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline;
using inexor::vulkan_renderer::wrapper::pipelines::PipelineCache;
using inexor::vulkan_renderer::wrapper::swapchains::Swapchain;
using inexor::vulkan_renderer::wrapper::windows::Mode;
using inexor::vulkan_renderer::wrapper::windows::Window;
using inexor::vulkan_renderer::wrapper::windows::WindowSurface;

namespace inexor::example_app {

/// Octree vertex struct
struct OctreeVertex {
    glm::vec3 position;
    glm::vec3 color;
};

/// The base class of the Inexor vulkan-renderer example app.
class ExampleAppBase {
protected:
    // Make sure to declare the instance before the debug utils messenger callback
    // to ensure the correct order of destruction.
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<VulkanDebugUtilsCallback> m_dbg_callback;
    std::unique_ptr<WindowSurface> m_surface;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<RenderGraph> m_render_graph;

    // RENDERGRAPH2
    std::shared_ptr<vulkan_renderer::render_graph::RenderGraph> m_render_graph2;
    std::weak_ptr<vulkan_renderer::render_graph::Buffer> m_vertex_buffer2;
    std::weak_ptr<vulkan_renderer::render_graph::Buffer> m_index_buffer2;
    std::weak_ptr<vulkan_renderer::render_graph::Texture> m_back_buffer2;
    std::weak_ptr<vulkan_renderer::render_graph::Texture> m_depth_buffer2;
    std::weak_ptr<vulkan_renderer::render_graph::GraphicsPass> m_graphics_pass2;
    std::weak_ptr<vulkan_renderer::render_graph::Buffer> m_mvp_matrix2;
    VkDescriptorSetLayout m_descriptor_set_layout2{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptor_set2{VK_NULL_HANDLE};
    std::weak_ptr<GraphicsPipeline> m_octree_pipeline2;
    std::shared_ptr<PipelineCache> m_pipeline_cache2;
    std::shared_ptr<Shader> m_vertex_shader2;
    std::shared_ptr<Shader> m_fragment_shader2;
    std::shared_ptr<Swapchain> m_swapchain2;

    void setup_render_graph();
    void recreate_swapchain();
    void render_frame();

    // @TODO Everything below needs to be abstracted further so that it's no longer
    // part of ExampleAppBase, meaning the rendergraph requires further abstraction.

    // @TODO Swapchains will be decoupled from rendergraph again in the future
    // The rendergraph will be able to handle an arbitrary number of windows and swapchains.

    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Window> m_window;
    TextureResource *m_back_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};
    std::vector<ResourceDescriptor> m_descriptors;
    std::vector<OctreeGpuVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;
    std::vector<Shader> m_shaders;
    bool m_vsync_enabled{false};
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<ImGUIOverlay> m_imgui_overlay;
    bool m_window_resized{false};
    FPSLimiter m_fps_limiter;
    std::vector<UniformBuffer> m_uniform_buffers;

public:
    ~ExampleAppBase();
};

} // namespace inexor::example_app
