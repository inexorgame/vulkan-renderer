#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <volk.h>

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <optional>
#include <tuple>
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

// Namespaces
using wrapper::Device;
using wrapper::Swapchain;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorSetLayoutCache;
using wrapper::descriptors::DescriptorSetUpdateBuilder;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineLayout;

/// A rendergraph is a generic solution for rendering architecture
/// This is based on Yuriy O'Donnell's talk "FrameGraph: Extensible Rendering Architecture in Frostbite" from GDC 2017
/// Also check out Hans-Kristian Arntzen's blog post "Render graphs and Vulkan - a deep dive" (2017) and
/// Adam Sawicki's talk "Porting your engine to Vulkan or DX12" (2018)
class RenderGraph {
private:
    /// The device wrapper
    Device &m_device;
    /// The swapchain wrapper
    Swapchain &m_swapchain;

    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    // ---------------------------------------------------------------------------------------------------------
    //  GRAPHICS PASSES
    // ---------------------------------------------------------------------------------------------------------
    /// The graphics pass builder of the rendergraph
    GraphicsPassBuilder m_graphics_pass_builder{};

    /// The function used by the rendergraph to to create the graphics passes
    using GraphicsPassCreateFunction = std::function<std::shared_ptr<GraphicsPass>(GraphicsPassBuilder &)>;

    /// The name of the graphics pass create function is associated by using a std::pair
    std::vector<GraphicsPassCreateFunction> m_graphics_pass_create_functions;

    /// The graphics passes used in the rendergraph
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;

    // ---------------------------------------------------------------------------------------------------------
    //  GRAPHICS PIPELINES
    // ---------------------------------------------------------------------------------------------------------
    /// The graphics pipeline builder of the rendergraph
    GraphicsPipelineBuilder m_graphics_pipeline_builder;

    // ---------------------------------------------------------------------------------------------------------
    //  DESCRIPTORS
    // ---------------------------------------------------------------------------------------------------------
    /// NOTE: In C++, the order of initialization of member variables in a class constructor is determined by the order
    /// of declaration in the class, not by the order of arguments in the constructor's initializer list! Also, do not
    /// reset descriptor set layout cache when calling reset method of the rendergraph!
    DescriptorSetLayoutCache m_descriptor_set_layout_cache;
    DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;

    DescriptorSetAllocator m_descriptor_set_allocator;
    DescriptorSetUpdateBuilder m_descriptor_set_update_builder;

    // ---------------------------------------------------------------------------------------------------------
    //  PIPELINES
    // ---------------------------------------------------------------------------------------------------------
    /// The callables which create the graphics pipelines used in the rendergraph
    using GraphicsPipelineCreateFunction = std::function<std::shared_ptr<GraphicsPipeline>(GraphicsPipelineBuilder &)>;

    /// The callables to create the graphics pipelines used in the rendergraph
    std::vector<GraphicsPipelineCreateFunction> m_pipeline_create_functions;

    // TODO: Support compute pipelines and compute passes
    /// The graphics pipelines used in the rendergraph
    /// This will be populated using m_on_graphics_pipeline_create_callables
    std::vector<std::shared_ptr<GraphicsPipeline>> m_graphics_pipelines;

    // ---------------------------------------------------------------------------------------------------------
    //  BUFFERS
    // ---------------------------------------------------------------------------------------------------------
    // The buffer resources of the rendergraph (vertex-, index-, and uniform buffers)
    // Note that we must keep the data as std::vector of std::unique_ptr in order to keep entries consistent
    std::vector<std::shared_ptr<Buffer>> m_buffers;

    // ---------------------------------------------------------------------------------------------------------
    //  TEXTURES
    // ---------------------------------------------------------------------------------------------------------
    // TODO: We could split it into internal and external textures actually...
    /// TODO: Explain how textures are treated equally here
    std::vector<std::shared_ptr<Texture>> m_textures;

    // ---------------------------------------------------------------------------------------------------------
    //  DESCRIPTORS
    // ---------------------------------------------------------------------------------------------------------
    // @note Descriptors are not associated with a pipeline or a pass inside of rendergraph
    std::vector<std::tuple<std::function<void(DescriptorSetLayoutBuilder &)>,
                           std::function<void(DescriptorSetAllocator &)>,
                           std::function<void(DescriptorSetUpdateBuilder &)>>>
        m_resource_descriptors;

    // TODO: Add @exception to documentation of other methods/code parts!

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic!
    void check_for_cycles();

    /// Create the buffers of every buffer resource in the rendergraph
    /// @param cmd_buf The command buffer to record into
    void create_buffers();

    void create_descriptor_set_layouts();

    /// Create the graphics passes
    void create_graphics_passes();

    /// Create the graphics pipelines
    void create_graphics_pipelines();

    /// Create the textures
    void create_textures();

    /// Determine the order of execution of the graphics passes by using depth first search (dfs) algorithm
    void determine_pass_order();

    /// Record the command buffer of a pass
    /// @param cmd_buf The command buffer to record the pass into
    /// @param pass The graphics pass to record the command buffer for
    /// @param is_first_pass ``true`` if this is the first pass in the graphics pass stack
    /// @param is_last_pass ``true`` if this is the last pass in the graphics pass stack
    /// @param img_index The swapchain image index
    void record_command_buffer_for_pass(const CommandBuffer &cmd_buf,
                                        const GraphicsPass &pass,
                                        bool is_first_pass,
                                        bool is_last_pass,
                                        std::uint32_t img_index);

    /// Record all command buffers required for the passes
    /// @param cmd_buf The command buffer to record all passes with
    /// @param img_index The swapchain image index
    void record_command_buffers(const CommandBuffer &cmd_buf, std::uint32_t img_index);

    /// Update the vertex-, index-, and uniform-buffers
    /// @note If a uniform buffer has been updated, an update of the associated descriptor set will be performed
    void update_buffers();

    /// Update dynamic textures
    void update_textures();

    /// Update the descriptor sets
    void update_descriptor_sets();

    /// Make sure all required resources are specified so rendergraph is ready to be compiled
    void validate_render_graph();

public:
    /// Default constructor
    /// @note device and swapchain are not taken as a const reference because rendergraph needs to modify both
    /// @param device The device wrapper
    /// @param swapchain The swapchain wrapper
    RenderGraph(Device &device, Swapchain &swapchain);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a new graphics pass to the rendergraph
    /// @param on_pass_create A callable to create the graphics pass using GraphicsPassBuilder
    /// @note Move semantics is used to std::move on_pass_create
    void add_graphics_pass(GraphicsPassCreateFunction on_pass_create);

    // TODO: One thread_local GraphicsPipelineBuilder? Once graphics pipelines will be created in parallel!

    /// Add a new graphics pipeline to the rendergraph
    /// @param on_pipeline_create A function to create the graphics pipeline using GraphicsPipelineBuilder
    /// @note Move semantics is used to std::move on_pipeline_create
    void add_graphics_pipeline(GraphicsPipelineCreateFunction on_pipeline_create);

    // TODO: Should those return a std::shared_ptr or a std::weak_ptr now? (Which memory ownership model?)

    /// Add an buffer to rendergraph
    /// @param name The name of the buffer
    /// @param on_update The update function of the index buffer
    /// @return A shared pointer to the buffer resource that was created
    [[nodiscard]] std::shared_ptr<Buffer>
    add_buffer(std::string buffer_name, BufferType buffer_type, std::function<void()> on_update);

    ///
    void allocate_descriptor_sets();

    /// Add a descriptor to rendergraph
    /// @param on_create_descriptor_set_layout
    /// @param on_allocate_descriptor_set
    /// @param on_update_descriptor_set
    void add_resource_descriptor(std::function<void(DescriptorSetLayoutBuilder &)> on_create_descriptor_set_layout,
                                 std::function<void(DescriptorSetAllocator &)> on_allocate_descriptor_set,
                                 std::function<void(DescriptorSetUpdateBuilder &)> on_update_descriptor_set);

    /// Add a texture which will be initialized inside of rendergraph (not outside of it)
    /// @param texture_name The name of the texture
    /// @param usage
    /// @param format
    /// @return
    [[nodiscard]] std::shared_ptr<Texture> add_texture(std::string texture_name, TextureUsage usage, VkFormat format);

    /// Add a texture which will be initialized externally (not inside of rendergraph)
    /// @param texture_name The name of the texture
    /// @param usage
    /// @param on_init
    /// @param on_update
    /// @return
    [[nodiscard]] std::shared_ptr<Texture> add_texture(std::string texture_name,
                                                       TextureUsage usage,
                                                       std::optional<std::function<void()>> on_init = std::nullopt,
                                                       std::optional<std::function<void()>> on_update = std::nullopt);

    // TODO: Keep track of internal state? What happens when calling render() before compiler()?
    /// Compile the rendergraph
    void compile();

    /// Render a frame
    void render();

    /// Reset the entire RenderGraph
    void reset();

    /// Update rendering data like buffers or textures
    void update_data();
};

} // namespace inexor::vulkan_renderer::render_graph
