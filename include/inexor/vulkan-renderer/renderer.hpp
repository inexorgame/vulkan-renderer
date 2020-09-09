﻿#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/frame_graph.hpp"
#include "inexor/vulkan-renderer/gpu_info.hpp"
#include "inexor/vulkan-renderer/imgui.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/resource_descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/texture.hpp"
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
    // We try to avoid inheritance here and prefer a composition pattern.
    // TODO: VulkanSwapchainManager, VulkanPipelineManager, VulkanRenderPassManager?

    std::shared_ptr<VulkanGraphicsCardInfoViewer> m_gpu_info_manager{new VulkanGraphicsCardInfoViewer};

    std::shared_ptr<AvailabilityChecksManager> m_availability_checks_manager{new AvailabilityChecksManager};

    std::shared_ptr<VulkanSettingsDecisionMaker> m_settings_decision_maker{new VulkanSettingsDecisionMaker};

    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

    VkDebugReportCallbackEXT m_debug_report_callback{};

    bool m_debug_report_callback_initialised{false};

    TimeStep m_time_step;

    std::uint32_t m_window_width{0};

    std::uint32_t m_window_height{0};

    std::string m_window_title;

    FPSCounter m_fps_counter;

    // TODO: Refactor this!
    VkDescriptorBufferInfo m_uniform_buffer_info{};

    bool m_vsync_enabled{false};

    Camera m_game_camera;

    std::unique_ptr<wrapper::GLFWContext> m_glfw_context;
    std::unique_ptr<wrapper::Window> m_window;
    std::unique_ptr<wrapper::Instance> m_instance;
    std::unique_ptr<wrapper::Device> m_device;
    std::unique_ptr<wrapper::WindowSurface> m_surface;
    std::unique_ptr<wrapper::Swapchain> m_swapchain;
    std::unique_ptr<wrapper::CommandPool> m_command_pool;
    std::unique_ptr<ImGUIOverlay> m_imgui_overlay;
    std::unique_ptr<wrapper::Semaphore> m_image_available_semaphore;
    std::unique_ptr<wrapper::Semaphore> m_rendering_finished_semaphore;
    std::unique_ptr<FrameGraph> m_frame_graph;

    std::vector<wrapper::Shader> m_shaders;
    std::vector<wrapper::Texture> m_textures;
    std::vector<wrapper::UniformBuffer> m_uniform_buffers;
    std::vector<wrapper::ResourceDescriptor> m_descriptors;
    std::vector<OctreeGpuVertex> m_octree_vertices;
    std::vector<std::uint16_t> m_octree_indices;

    void setup_frame_graph();
    void generate_octree_indices();

    void recreate_swapchain();

    void render_frame();

public:
    ~VulkanRenderer();

    /// @brief Run Vulkan memory allocator's memory statistics
    void calculate_memory_budget();

    bool m_window_resized{false};

    /// Neccesary for taking into account the relative speed of the system's CPU.
    float m_time_passed{0.0f};

    ///
    TimeStep m_stopwatch;
};

} // namespace inexor::vulkan_renderer
