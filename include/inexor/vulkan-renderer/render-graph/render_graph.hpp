#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/render-modules/render_module_base.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <spdlog/spdlog.h>
#include <volk.h>

#include <functional>
#include <memory>
#include <optional>
#include <stack>
#include <tuple>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declarations
class Device;
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class GraphicsPass;

// Using declarations
using wrapper::Device;
using wrapper::Swapchain;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorSetLayoutCache;
using wrapper::descriptors::WriteDescriptorSetBuilder;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineCache;
using wrapper::pipelines::PipelineLayout;

/// A rendergraph is a generic solution for rendering architecture.
class RenderGraph {
private:
    /// The device wrapper
    Device &m_device;

    /// The graphics pass builder of the rendergraph
    GraphicsPassBuilder m_graphics_pass_builder;

    /// The unique wait semaphores of all swapchains used (This means if one swapchain is used mutliple times it's still
    /// only one VkSemaphore in here because collect_swapchain_image_available_semaphores method will fill this vector)
    std::vector<VkSemaphore> m_swapchains_imgs_available;

    // Some textures and buffers are owned by rendergraph, some are owned by render modules!
    std::vector<std::shared_ptr<BufferResource>> m_buffers;
    std::vector<std::shared_ptr<TextureResource>> m_textures;

    // TODO: Support compute pipelines

    /// The graphics pipeline builder of the rendergraph
    GraphicsPipelineBuilder m_graphics_pipeline_builder;

    /// The descriptor set layout builder (a builder pattern for descriptor set layouts)
    DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;

    /// The descriptor set allocator
    DescriptorSetAllocator m_descriptor_set_allocator;

    /// The write descriptor set builder (a builder pattern for write descriptor sets)
    WriteDescriptorSetBuilder m_write_descriptor_set_builder;

    const PipelineCache &m_pipeline_cache;

    /// The individual modules of the renderer
    std::vector<std::shared_ptr<render_modules::RenderModuleBase>> m_render_modules;

    /// Allocate the descriptor sets
    void allocate_descriptor_sets();

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic!
    void check_for_cycles();

    /// Collect all image available semaphores of all swapchains which are used into one std::vector<VkSemaphore>
    void collect_swapchain_image_available_semaphores();

    /// Create the descriptor set layouts
    void create_descriptor_set_layouts();

    /// Create the graphics pipelines
    void create_graphics_pipelines();

    /// Determine the order of execution of the graphics passes by using depth first search (DFS) algorithm
    void determine_pass_order();

    /// Fill the VkRenderingInfo for a graphics pass
    /// @param pass The graphics pass
    void fill_graphics_pass_rendering_info(GraphicsPass &pass);

    // TODO: Record command buffers in parallel!
    void record_and_submit_command_buffers();

    /// Record the command buffer of a pass. After a lot of discussions about the API design of rendergraph, we came to
    /// the conclusion that it's the full responsibility of the programmer to manually bind pipelines, descriptors sets,
    /// and buffers inside of the on_record function instead of attempting to abstract all of this in rendergraph. This
    /// means rendergraph will not automatically bind pipelines, buffers, or descriptor sets! The reason for this is
    /// that there could be complex rendering going on inside of the on_record function with an arbitrary number of
    /// pipelines descriptor sets, and buffers being bound in a nontrivial order or under conditional cases. We then
    /// refrained from designing a simple API inside of rendergraph which automatically binds one graphics pipeline,
    /// descriptor set, or a set of buffers at the beginning of rendering before calling on_record because it would
    /// cause confusion about the correct API usage for the advanced use cases. Nonetheless, the creation of buffers,
    /// descriptors, or pipelines is still the full responsibility of the rendergraph, but you need to use them manually
    /// inside of the on_record function.
    /// @param cmd_buf The command buffer to record the pass into
    /// @param pass The graphics pass to record the command buffer for
    void record_command_buffer_for_pass(const CommandBuffer &cmd_buf, const GraphicsPass &pass);

    /// Update the vertex-, index-, and uniform-buffers
    /// @note If a uniform buffer has been updated, an update of the associated descriptor set will be performed
    void update_buffers();

    /// Update dynamic textures
    void update_textures();

    /// Update the write descriptor sets
    /// We batch all descriptor set updates off all render modules into one big std::vector of write descriptor sets and
    /// call vkUpdateDescriptorSets only once per frame with it for improved performance.
    void update_write_descriptor_sets();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The pipeline cache
    RenderGraph(Device &device, const PipelineCache &pipeline_cache);

    void add_buffer(std::shared_ptr<BufferResource> buffer) {
        m_buffers.emplace_back(std::move(buffer));
    }

    void add_texture(std::shared_ptr<TextureResource> texture) {
        m_textures.emplace_back(std::move(texture));
    }

    /// Compile the rendergraph
    void compile();

    /// Render a frame
    void render();

    /// Reset the entire RenderGraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
