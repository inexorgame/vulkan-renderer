#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <volk.h>

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declarations
class Device;
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Forward declarations
enum class BufferType;
class PushConstantRangeResource;

// Namespaces
using namespace wrapper::pipelines;

/// A rendergraph is a generic solution for rendering architecture
/// This is based on Yuriy O'Donnell's talk "FrameGraph: Extensible Rendering Architecture in Frostbite" from GDC 2017
/// Also check out Hans-Kristian Arntzen's blog post "Render graphs and Vulkan - a deep dive" (2017) and
/// Adam Sawicki's talk "Porting your engine to Vulkan or DX12" (2018)
class RenderGraph {
private:
    /// The device wrapper
    wrapper::Device &m_device;
    /// The swapchain wrapper
    wrapper::Swapchain &m_swapchain;

    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    // ---------------------------------------------------------------------------------------------------------
    //  GRAPHICS PASSES
    // ---------------------------------------------------------------------------------------------------------
    /// The graphics pass builder of the rendergraph
    GraphicsPassBuilder m_graphics_pass_builder{};

    /// The callables which create the graphics passes used in the rendergraph
    using GraphicsPassCreateCallable =
        std::function<std::shared_ptr<render_graph::GraphicsPass>(render_graph::GraphicsPassBuilder &)>;

    /// The callables to create the graphics passes used in the rendergraph
    std::vector<GraphicsPassCreateCallable> m_on_graphics_pass_create_callables;

    /// The graphics passes used in the rendergraph
    /// This will be populated using m_on_graphics_pass_create_callables
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;

    // ---------------------------------------------------------------------------------------------------------
    //  GRAPHICS PIPELINES
    // ---------------------------------------------------------------------------------------------------------
    /// The graphics pipeline builder of the rendergraph
    wrapper::pipelines::GraphicsPipelineBuilder m_graphics_pipeline_builder;

    /// The callables which create the graphics pipelines used in the rendergraph
    using GraphicsPipelineCreateCallable = std::function<std::shared_ptr<GraphicsPipeline>(
        wrapper::pipelines::GraphicsPipelineBuilder &, const VkPipelineLayout)>;

    /// The callables to create the graphics pipelines used in the rendergraph
    std::vector<GraphicsPipelineCreateCallable> m_on_graphics_pipeline_create_callables;

    std::vector<std::unique_ptr<PipelineLayout>> m_graphics_pipeline_layouts;

    /// The graphics pipelines used in the rendergraph
    /// This will be populated using m_on_graphics_pipeline_create_callables
    std::vector<std::shared_ptr<GraphicsPipeline>> m_graphics_pipelines;

    // TODO: Support compute pipelines and compute passes

    // ---------------------------------------------------------------------------------------------------------
    //  BUFFERS AND TEXTURES
    // ---------------------------------------------------------------------------------------------------------
    // The buffer resources of the rendergraph (vertex-, index-, and uniform buffers)
    // Note that we must keep the data as std::vector of std::unique_ptr in order to keep entries consistent
    std::vector<std::shared_ptr<Buffer>> m_buffers;

    /// The push constant resources of the rendergraph
    // TODO: Remember we need to squash all VkPushConstantRange of a pass into one std::vector in order to bind it!
    // TODO: Should push constant ranges be per graphics pipeline?
    std::vector<std::shared_ptr<PushConstantRangeResource>> m_push_constant_ranges;

    /// The texture resources of the rendergraph
    std::vector<std::shared_ptr<Texture>> m_textures;

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic
    void check_for_cycles();

    /// Create the buffers of every buffer resource in the rendergraph
    void create_buffers();

    /// Descriptor management
    // TODO: better naming? create_descriptors?
    void create_descriptor_sets();

    /// Create the graphics passes
    /// @note This must happen before the graphics pipeline layouts can be created in
    /// ``create_graphics_pipeline_layouts()``
    void create_graphics_passes();

    /// Create the graphics pipeline layouts
    /// @note This must happen before the graphics pipelines can be created in ``create_graphics_pipeline_layouts()``
    void create_graphics_pipeline_layouts();

    /// Create the graphics pipelines
    void create_graphics_pipelines();

    /// Create the textures of every texture resoruce in the rendergraph
    void create_textures();

    /// Determine the order of execution of the graphics passes by using depth first search (dfs) algorithm
    void determine_pass_order();

    /// Record the command buffer of a pass
    /// @param cmd_buf The command buffer to record the pass into
    /// @param pass The graphics pass to record the command buffer for
    /// @param is_first_pass ``true`` if this is the first pass in the graphics pass stack
    /// @param is_last_pass ``true`` if this is the last pass in the graphics pass stack
    /// @param img_index The swapchain image index
    void record_command_buffer_for_pass(const wrapper::commands::CommandBuffer &cmd_buf, const GraphicsPass &pass,
                                        bool is_first_pass, bool is_last_pass, std::uint32_t img_index);

    /// Record all command buffers required for the passes
    /// @param cmd_buf The command buffer to record all passes with
    /// @param img_index The swapchain image index
    void record_command_buffers(const wrapper::commands::CommandBuffer &cmd_buf, std::uint32_t img_index);

    /// Update the vertex-, index-, and uniform-buffers
    /// @note If a uniform buffer has been updated, an update of the associated descriptor set will be performed
    void update_buffers();

    /// Update dynamic textures
    void update_textures();

    /// Update the descriptor sets
    void update_descriptor_sets();

    /// Update the push constant ranges
    void update_push_constant_ranges();

    /// Validate rendergraph
    /// @note For rendergraph validation, the passes of the rendergraph must already be created
    void validate_render_graph();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param swapchain The swapchain wrapper
    RenderGraph(wrapper::Device &device, wrapper::Swapchain &swapchain);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a buffer (vertex, index, or uniform buffer) resource to the rendergraph
    /// @param name The internal name of the buffer resource (must not be empty)
    /// @param type The internal buffer usage of the buffer
    /// @param on_update An optional buffer resource update function (``std::nullopt`` by default)
    /// @note Not every buffer must have an update function because index buffers should be updated with vertex buffers
    /// @exception std::runtime_error Internal debug name is empty
    /// @return A weak pointer to the buffer resource that was just created
    [[nodiscard]] std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type,
                                                   std::optional<std::function<void()>> on_update = std::nullopt);

    /// Add a new graphics pass to the rendergraph
    /// @param on_pass_create A callable to create the graphics pass using GraphicsPassBuilder
    void add_graphics_pass(GraphicsPassCreateCallable on_pass_create) {
        m_on_graphics_pass_create_callables.emplace_back(std::move(on_pass_create));
    }

    /// Add a new graphics pipeline to the rendergraph
    /// @param on_pipeline_create A callable to create the graphics pipeline using GraphicsPipelineBuilder
    void add_graphics_pipeline(GraphicsPipelineCreateCallable on_pipeline_create) {
        // TODO: Can this be emplace_back?
        m_on_graphics_pipeline_create_callables.push_back(std::move(on_pipeline_create));
    }

    /// Add a push constant range resource to the rendergraph
    /// @tparam T The data type of the push constant range
    /// @param data A pointer to the data of the push constant range
    /// @param on_update The update function of the push constant range
    /// @param stage_flags The shader stage flags
    /// @param offset The offset in bytes (``0`` by default)
    /// @return The this pointer, allowing for methods to be chained as a builder pattern
    template <typename PushConstantDataType>
    void add_push_constant_range(
        const PushConstantDataType *data, std::function<void()> on_update = []() {},
        const VkShaderStageFlags stage_flags = VK_SHADER_STAGE_VERTEX_BIT, const std::uint32_t offset = 0) {
        m_push_constant_ranges.emplace_back(
            VkPushConstantRange{
                .stageFlags = stage_flags,
                .offset = offset,
                .size = sizeof(PushConstantDataType),
            },
            data, std::move(on_update));
    }

    /// Add a texture resource to the rendergraph
    /// @param name The name of the texture (must not be empty)
    /// @param uage The texture usage inside of rendergraph
    /// @param format The image format of the texture
    /// @param on_init The initialization callable of the texture
    /// @param on_update The update callable of the texture
    /// @return A weak pointer to the texture resource that was created
    [[nodiscard]] std::weak_ptr<Texture> add_texture(std::string name, TextureUsage usage, VkFormat format,
                                                     std::optional<std::function<void()>> on_init = std::nullopt,
                                                     std::optional<std::function<void()>> on_update = std::nullopt);

    /// Compile the entire rendergraph
    void compile();

    /// Render a frame
    void render();

    /// Update all the rendering data (buffers, textures...)
    void update_data();
};

} // namespace inexor::vulkan_renderer::render_graph
