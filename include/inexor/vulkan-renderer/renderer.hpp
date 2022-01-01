#pragma once

#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/cubemap/cubemap_generator.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_cpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_pbr_renderer.hpp"
#include "inexor/vulkan-renderer/imgui.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/pbr/brdf_lut.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_cpu_data.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_renderer.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"
#include "inexor/vulkan-renderer/vk_tools/gpu_info.hpp"
#include "inexor/vulkan-renderer/world/octree_cpu_data.hpp"
#include "inexor/vulkan-renderer/world/octree_gpu_data.hpp"
#include "inexor/vulkan-renderer/world/octree_renderer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
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

    VkDebugReportCallbackEXT m_debug_report_callback{VK_NULL_HANDLE};

    bool m_debug_report_callback_initialised{false};
    bool m_vsync_enabled{false};

    TimeStep m_time_step;

    std::uint32_t m_window_width{0};
    std::uint32_t m_window_height{0};
    wrapper::Window::Mode m_window_mode{wrapper::Window::Mode::WINDOWED};

    std::string m_window_title;

    FPSCounter m_fps_counter;

    std::vector<std::string> m_texture_files;

    std::unique_ptr<Camera> m_camera;

    std::unique_ptr<wrapper::Window> m_window;
    std::unique_ptr<wrapper::Instance> m_instance;
    std::unique_ptr<wrapper::Device> m_device;
    std::unique_ptr<wrapper::WindowSurface> m_surface;
    std::unique_ptr<wrapper::Swapchain> m_swapchain;
    std::unique_ptr<wrapper::CommandPool> m_command_pool;
    std::unique_ptr<ImGUIOverlay> m_imgui_overlay;
    std::unique_ptr<wrapper::Fence> m_frame_finished_fence;
    std::unique_ptr<wrapper::Semaphore> m_image_available_semaphore;
    std::unique_ptr<RenderGraph> m_render_graph;

    std::vector<std::string> m_gltf_model_file_names;
    std::vector<gltf::ModelCpuData> m_gltf_cpu_data;
    std::vector<gltf::ModelGpuData> m_gltf_gpu_data;
    std::unique_ptr<gltf::ModelRenderer> m_gltf_model_renderer;

    std::unique_ptr<pbr::BRDFLUTGenerator> m_pbr_brdf_lut;
    std::unique_ptr<cubemap::CubemapGenerator> m_cubemap;

    std::unique_ptr<texture::CpuTexture> m_env_cube;
    std::unique_ptr<texture::GpuTexture> m_env_cube_texture;

    std::vector<std::string> m_octree_vertex_shader_files;
    std::vector<std::string> m_octree_fragment_shader_files;
    std::vector<wrapper::Shader> m_octree_shaders;

    std::vector<std::shared_ptr<world::Cube>> m_worlds;
    std::vector<world::OctreeCpuData<OctreeGpuVertex>> m_octree_cpu_data;
    std::vector<world::OctreeGpuData<UniformBufferObject, OctreeGpuVertex>> m_octree_gpu_data;

    std::unique_ptr<world::OctreeRenderer<OctreeGpuVertex>> m_octree_renderer;
    std::vector<texture::GpuTexture> m_textures;

    std::unique_ptr<skybox::SkyboxCpuData> m_skybox_cpu_data;
    std::unique_ptr<skybox::SkyboxGpuData> m_skybox_gpu_data;
    std::unique_ptr<skybox::SkyboxRenderer> m_skybox_renderer;

    TextureResource *m_back_buffer{nullptr};
    TextureResource *m_depth_buffer{nullptr};

    struct DescriptorSetLayouts {
        VkDescriptorSetLayout scene;
        VkDescriptorSetLayout material;
        VkDescriptorSetLayout node;
    } descriptorSetLayouts;

    struct DescriptorSets {
        VkDescriptorSet scene;
        VkDescriptorSet skybox;
    };

    std::vector<DescriptorSets> descriptorSets;

    struct Textures {
        std::unique_ptr<texture::GpuTexture> environmentCube;
        std::unique_ptr<texture::GpuTexture> empty;
        std::unique_ptr<texture::GpuTexture> irradianceCube;
        std::unique_ptr<texture::GpuTexture> prefilteredCube;
        std::unique_ptr<pbr::BRDFLUTGenerator> lutBrdf;
    } textures;

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;

    void setup_render_graph();
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
