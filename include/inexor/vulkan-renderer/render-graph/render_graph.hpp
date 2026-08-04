#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_copy_batch_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/frame_sync_manager.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/resource_descriptor_manager.hpp"
#include "inexor/vulkan-renderer/render-graph/staging_buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/swapchain_manager.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_copy_batch_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/queries/query_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.hpp"

#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::synchronization {
// Forward declaration
class Semaphore;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class Buffer;
class GraphicsPass;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::render_graph {

// Using declarations
using wrapper::commands::CommandBufferCache;
using wrapper::core::DebugLabelColor;
using wrapper::core::Device;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineCache;
using wrapper::synchronization::PipelineBarrierBuilder;

// @TODO How to handle optional texture update depending on texture type?
// @TODO By implementing textures which are not updated, but only initliazed, we could save memory!
class RenderGraph {
private:
    // The device wrapper
    Device &m_device;

    /// --------------------------------------------------------------------------------------------------
    /// BUFFERS
    /// --------------------------------------------------------------------------------------------------

    /// The vertex buffers, index buffers, and uniform buffers
    std::vector<std::shared_ptr<Buffer>> m_buffers;

    /// --------------------------------------------------------------------------------------------------
    /// TEXTURES
    /// --------------------------------------------------------------------------------------------------

    /// The textures, back buffers, and depth buffers
    std::vector<std::shared_ptr<Texture>> m_textures;

    /// The resource descriptor manager of the rendergraph
    ResourceDescriptorManager m_resource_descriptors;

    /// --------------------------------------------------------------------------------------------------
    /// GRAPHICS PIPELINES
    /// --------------------------------------------------------------------------------------------------

    /// The graphics pipeline builder
    GraphicsPipelineBuilder m_graphics_pipeline_builder;
    /// A using declaration for graphics pipeline create functions
    using OnBuildGraphicsPipeline = std::function<void(GraphicsPipelineBuilder &)>;
    /// The graphics pipeline create functions registered to the rendergraph
    std::vector<OnBuildGraphicsPipeline> m_graphics_pipeline_create_functions;

    /// --------------------------------------------------------------------------------------------------
    /// GRAPHICS PASSES
    /// --------------------------------------------------------------------------------------------------

    /// The graphics pass builder
    GraphicsPassBuilder m_graphics_pass_builder;
    // A using declaration for graphics pass create functions
    using OnBuildGraphicsPass = std::function<std::shared_ptr<GraphicsPass>(GraphicsPassBuilder &)>;
    /// The graphics passes registered to the rendergraph
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;

    /// --------------------------------------------------------------------------------------------------

    SwapchainManager m_swapchain_manager;
    CommandBufferCache m_command_buffer_cache;
    std::unique_ptr<wrapper::queries::QueryPool> m_query_pool;
    float m_timestamp_period{0.0f};
    std::unique_ptr<wrapper::synchronization::Semaphore> m_upload_finished;
    bool m_upload_submission_pending{false};
    VkPipelineStageFlags2 m_upload_wait_stage_mask{VK_PIPELINE_STAGE_2_NONE};
    std::function<void(wrapper::commands::CommandBufferBuilder &)> m_inline_update_commands;
    std::vector<std::function<void()>> m_inline_update_pending_releases;

    /// Queue family ownership transfer barriers to be replayed as "acquire" operations on the graphics queue,
    /// matching the "release" barriers already recorded on the transfer queue submission. Only populated when
    /// buffer/texture updates were uploaded via a dedicated transfer queue whose family differs from the graphics
    /// queue family (VK_SHARING_MODE_EXCLUSIVE resources require an explicit ownership transfer in that case).
    PipelineBarrierBatchBuilder m_pending_queue_ownership_acquire_barriers;
    BufferCopyBatchBuilder m_buffer_copy_batch_builder;
    TextureCopyBatchBuilder m_texture_copy_batch_builder;
    StagingBuffer m_staging_buffer;
    FrameSyncManager m_frame_sync_manager;
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    std::vector<PendingBufferCopy> m_scratch_pending_buffer_copies;
    std::vector<PendingTextureCopy> m_scratch_pending_texture_copies;
    std::vector<std::function<void()>> m_scratch_pending_releases;
    std::vector<VkFormat> m_scratch_color_attachment_formats;
    /// Reused scratch storage for render() to avoid a heap allocation every frame
    std::vector<wrapper::core::QueueSemaphoreWait> m_scratch_render_wait_semaphores;

    void defer_release(std::span<const VkFence> fences, std::function<void()> release);

    void synchronize_frame_context();

    void invalidate_graphics_pass_secondary_cmd_buffers();
    void invalidate_graphics_passes_using_texture(const Texture &texture);

    /// --------------------------------------------------------------------------------------------------

    /// @TODO Implement!
    void sort_graphics_passes_by_order();

    /// Update textures and buffers
    void update_resources();

    /// Create the graphics pipelines
    void create_graphics_pipelines();

    /// Build the cached texture-to-graphics-pass dependencies used for invalidation.
    void build_texture_graphics_pass_dependencies();

    /// Ensure that rendergraph is a directed acyclic graph (DAG)
    void check_for_cycles();

    /// Rebuild the static texture attachment part of VkRenderingInfo for a graphics pass.
    /// @param pass The graphics pass
    void rebuild_graphics_pass_texture_rendering_info(GraphicsPass &pass);

    /// Refresh the per-frame swapchain attachment part of VkRenderingInfo for a graphics pass.
    /// @param pass The graphics pass
    void refresh_graphics_pass_swapchain_rendering_info(GraphicsPass &pass);

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
    void record_command_buffer_for_pass(const wrapper::commands::CommandBuffer &cmd_buf, GraphicsPass &pass);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param use_secondary_command_buffers Whether graphics passes should be recorded into cached secondary command
    /// buffers or directly into the primary command buffer.
    RenderGraph(Device &device, bool use_secondary_command_buffers = true);

    ~RenderGraph();

    /// Add a buffer to the rendergraph
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    /// @param update_mode How the buffer should be updated and allocated
    [[nodiscard]] std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type, std::function<void()> on_update,
                                                   BufferUpdateMode update_mode = BufferUpdateMode::DEVICE_LOCAL);

    /// Add a graphics pass to the rendergraph
    /// @param graphics_pass The graphics pass which was created
    /// @note There is no name parameter here because the OnBuildGraphicsPass callback will use GraphicsPassBuilder to
    /// build a GraphicsPass, and inside of the builder the name will be set already.
    /// @return A weak pointer to the graphics pass which was created
    [[nodiscard]] std::weak_ptr<GraphicsPass> add_graphics_pass(OnBuildGraphicsPass on_build_graphics_pass);

    /// Add a graphics pipeline to rendergraph
    /// @param on_build_graphics_pipeline The graphics pipeline which will be created
    /// @note There is no name parameter here because the OnCreateGraphicsPipeline callback will use
    /// GraphicsPipelineBuilder to build a GraphicsPipeline, and inside of the builder the name will be set already.
    /// @note This API was chosen so inside of the OnCreateGraphicsPipeline lambda, we build the graphics pipeline with
    /// the help of the rendergraph's graphics pipeline builder and store the created graphics pipeline in the renderer
    /// class which uses this method. The created graphics pipeline is stored via reference capture of the lambda.
    /// An alternative design approach would be to somehow use pointers to smart pointers, passing the smart pointer to
    /// OnCreateGraphicsPipeline lambda, which seemed like a design flaw to us. We could write wrappers for this, but
    /// there is a tradeoff between simplicity of the code by not using such a wrapper compared with the simplicity of
    /// code by not using these lambdas. Also, if we would use such wrappers (or pointers to smart pointer), we would
    /// make object lifetime even more complex, which we should avoid at all cost.
    void add_graphics_pipeline(OnBuildGraphicsPipeline on_build_graphics_pipeline);

    /// Add a descriptor-backed render-graph resource and create the matching descriptor set layout/write updates
    /// @param resource The buffer or texture resource to bind
    /// @param stage The shader stage flag for the descriptor binding
    /// @param dst_binding The destination binding for the descriptors
    /// @return A weak pointer to the created
    /// per-frame descriptor set wrapper
    /// @note Buffer resources must be uniform buffers
    /// @note Texture resources are bound as combined image samplers
    [[nodiscard]] std::weak_ptr<PerFrameDescriptorSets>
    add_resource_descriptor(std::variant<std::weak_ptr<Buffer>, std::weak_ptr<Texture>> resource,
                            VkShaderStageFlags stage, std::uint32_t dst_binding = 0);

    /// Add a texture to the rendergraph
    /// @param name The texture name
    /// @param usage The texture usage
    /// @param format The texture format
    /// @param width The texture width
    /// @param height The texture height
    /// @param channels The number of channels
    /// @param sample_count The number of samples
    /// @param on_update The texture update function
    /// @return A weak pointer to the texture which was created
    [[nodiscard]] std::weak_ptr<Texture> add_texture(std::string name, TextureUsage usage, VkFormat format,
                                                     std::uint32_t width, std::uint32_t height,
                                                     std::uint32_t channels = 1,
                                                     VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
                                                     std::optional<std::function<void()>> on_update = std::nullopt);

    /// Compile the rendergraph
    /// Ideally, this should only be done once at startup and all changes in the system will be reported to
    /// rendergraph.
    void compile();

    /// Since we need to pass the rendergraph to every render module anyways,
    /// there is no need to pass the device wrapper every time as well
    [[nodiscard]] const auto &device() const {
        return m_device;
    }

    /// Render a frame while dealing automatically with all frames in flight internally
    void render();

    /// Log the most recently recorded GPU frame time.
    void log_gpu_frame_time() const;

    /// Reset the entire rendergraph
    /// @note We avoid to name it reset() because this would be ambiguous with smart pointer methods
    void reset_graph();
};

} // namespace inexor::vulkan_renderer::render_graph
