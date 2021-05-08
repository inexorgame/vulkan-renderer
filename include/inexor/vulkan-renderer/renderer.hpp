#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/imgui.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"
#include "inexor/vulkan-renderer/vk_tools/gpu_info.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"
#include "inexor/vulkan-renderer/wrapper/window_surface.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer {

class VulkanRenderer {
protected:
    std::shared_ptr<VulkanSettingsDecisionMaker> m_settings_decision_maker{
        std::make_shared<VulkanSettingsDecisionMaker>()};

    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

    VkDebugReportCallbackEXT m_debug_report_callback{VK_NULL_HANDLE};

    bool m_debug_report_callback_initialised{false};

    TimeStep m_time_step;

    std::uint32_t m_window_width{0};
    std::uint32_t m_window_height{0};
    wrapper::Window::Mode m_window_mode{wrapper::Window::Mode::WINDOWED};

    std::string m_window_title;

    FPSCounter m_fps_counter;

    bool m_vsync_enabled{false};

    std::unique_ptr<Camera> m_camera;

    std::unique_ptr<wrapper::GLFWContext> m_glfw_context;
    std::unique_ptr<wrapper::Window> m_window;
    std::unique_ptr<wrapper::Instance> m_instance;
    std::unique_ptr<wrapper::Device> m_device;
    std::unique_ptr<wrapper::WindowSurface> m_surface;
    std::unique_ptr<wrapper::Swapchain> m_swapchain;
    std::unique_ptr<wrapper::CommandPool> m_command_pool;
    std::unique_ptr<ImGUIOverlay> m_imgui_overlay;
    std::unique_ptr<wrapper::Fence> m_frame_finished_fence;
    std::unique_ptr<wrapper::Semaphore> m_image_available_semaphore;
    std::unique_ptr<wrapper::Semaphore> m_rendering_finished_semaphore;
    std::unique_ptr<RenderGraph> m_render_graph;

    std::vector<wrapper::Shader> m_shaders;
    std::vector<wrapper::GpuTexture> m_textures;
    std::vector<wrapper::UniformBuffer> m_uniform_buffers;
    std::vector<wrapper::ResourceDescriptor> m_descriptors;
    std::vector<OctreeGpuVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;

    // Render graph buffers for octree geometry.
    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};

    void setup_render_graph();
    void generate_octree_indices();
    void recreate_swapchain();
    void render_frame();

public:
    VulkanRenderer() = default;
    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    ~VulkanRenderer();

    VulkanRenderer &operator=(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;

    /// @brief Run Vulkan memory allocator's memory statistics
    void calculate_memory_budget();

    bool m_window_resized{false};

    /// Necessary for taking into account the relative speed of the system's CPU.
    float m_time_passed{0.0f};

    ///
    TimeStep m_stopwatch;
};

} // namespace inexor::vulkan_renderer
