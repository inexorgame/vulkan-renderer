#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class WriteDescriptorSetBuilder;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class GraphicsPassBuilder;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::pipelines

namespace inexor::vulkan_renderer {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_modules {

// Using declarations
using render_graph::BufferResource;
using render_graph::CommandBuffer;
using render_graph::GraphicsPass;
using render_graph::GraphicsPassBuilder;
using render_graph::RenderGraph;
using render_graph::Texture;
using wrapper::Device;
using wrapper::Shader;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayout;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;

/// An abstract base class for render modules in the rendergraph
class RenderModuleBase {
    friend RenderGraph;

private:
    std::string m_name;

protected:
    const Device &m_device;

    // TODO: Is this a good idea? We can abstract rendergraph on top of this.
    std::vector<VkSemaphore> m_semaphores;

    std::vector<GraphicsPass> m_graphics_passes;
    std::vector<Shader> m_shaders;
    std::vector<GraphicsPipeline> m_graphics_pipelines;
    std::vector<DescriptorSetLayout> m_descriptor_set_layouts;
    std::vector<std::shared_ptr<BufferResource>> m_buffers;
    std::vector<std::shared_ptr<TextureResource>> m_textures;

    // The following pure virtual methods must be implemented by every class which inherits from RenderModuleBase! This
    // also applies to some methods which are optional for some render modules like update_buffers or update_textures.
    // In these cases, it's still better to enforce the programmer to explicitely implement an empty method when writing
    // a render module which inherits from RenderModuleBase.
    virtual void setup_buffers() = 0;
    virtual void setup_textures() = 0;
    virtual void setup_graphics_passes(GraphicsPassBuilder &builder) = 0;
    virtual void setup_graphics_pipelines(GraphicsPipelineBuilder &builder) = 0;
    virtual void setup_shaders() = 0;
    virtual void setup_descriptor_set_layouts(DescriptorSetLayoutBuilder &builder) = 0;
    virtual void allocate_descriptor_sets(DescriptorSetAllocator &allocator) = 0;
    virtual void update_descriptor_sets(WriteDescriptorSetBuilder &builder) = 0;
    // TODO: Implementing it like this means we can only update buffers on a per-rendermodule basis unless the
    // update_buffers implements some internal parallelization as well! Once we implement taskflow, we could pass the
    // taskflow wrapper to update_buffers() though.
    virtual void update_buffers() = 0;
    virtual void update_textures() = 0;

    // NOTE: Recording command buffers should be done on a per-pass instead of a per-rendermodule basis for granularity.
    // This is the reason there is no pure virtual method like setup_command_buffer_recording here!

public:
    /// Default constructor
    /// @param device A const reference to the device wrapper
    /// @param rendergraph A const reference to the rendergraph wrapper
    /// @param name The name of the render module
    RenderModuleBase(const Device &device, std::string name);

    virtual ~RenderModuleBase() = default;

    // TODO: How to handle a pass reading from another pass?
};

} // namespace inexor::vulkan_renderer::render_modules
