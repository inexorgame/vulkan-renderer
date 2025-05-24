#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/render-modules/render_module_base.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace inexor::vulkan_renderer::render_modules {

// Using declarations
using render_graph::BufferResource;
using render_graph::BufferType;
using render_graph::RenderGraph;
using render_graph::TextureResource;
using vulkan_renderer::tools::Camera;
using world::Cube;
using wrapper::DebugLabelColor;
using wrapper::Device;
using wrapper::GraphicsPass;
using wrapper::Shader;
using wrapper::Swapchain;
using wrapper::commands::CommandBuffer;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineLayout;

/// A rendering class for octrees
class OctreeRenderer : public RenderModuleBase {
private:
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};
    std::vector<std::weak_ptr<Cube>> m_cubes;
    VkFormat m_back_buffer_img_format;
    VkExtent2D m_back_buffer_extent;
    glm::mat4 m_model_matrix{};
    std::weak_ptr<Camera> m_camera;
    std::weak_ptr<TextureResource> m_back_buffer;
    std::weak_ptr<TextureResource> m_depth_buffer;
    std::shared_ptr<BufferResource> m_camera_matrix;

    // NOTE: These methods must be implemented by every render module which inherits from RenderModuleBase!
    void setup_buffers() override;
    void setup_textures() override;
    void setup_graphics_passes(GraphicsPassBuilder &builder) override;
    void setup_graphics_pipelines(GraphicsPipelineBuilder &builder) override;
    void setup_shaders() override;
    void setup_descriptor_set_layouts(DescriptorSetLayoutBuilder &builder) override;
    void allocate_descriptor_sets(DescriptorSetAllocator &allocator) override;
    void update_descriptor_sets(WriteDescriptorSetBuilder &builder) override;
    void update_buffers() override;
    void update_textures() override;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param back_buffer The back buffer to render to
    /// @param depth_buffer The depth buffer to render to
    OctreeRenderer(const Device &device,
                   std::weak_ptr<TextureResource> back_buffer,
                   std::weak_ptr<TextureResource> depth_buffer);

    /// Add a new cube to the octree renderer
    /// @param cube The root cube to add to the renderer
    void add_octree(std::weak_ptr<Cube> cube);

    /// Remove a cube from the octree renderer
    /// @param cube The cube to remove
    void remove_octree(std::weak_ptr<Cube> cube);

    /// Set the camera used as view matrix for octree rendering
    /// @param camera The new camera to use for rendering
    void set_camera(std::weak_ptr<Camera> camera);
};

} // namespace inexor::vulkan_renderer::render_modules
