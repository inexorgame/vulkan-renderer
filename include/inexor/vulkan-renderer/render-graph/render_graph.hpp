#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/shader.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
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

// Namespaces
using wrapper::Device;
using wrapper::Swapchain;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorSetLayoutCache;
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
    std::vector<std::pair<std::string, GraphicsPassCreateFunction>> m_graphics_pass_create_functions;

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

    // ---------------------------------------------------------------------------------------------------------
    //  PIPELINES
    // ---------------------------------------------------------------------------------------------------------
    /// The callables which create the graphics pipelines used in the rendergraph
    using GraphicsPipelineCreateFunction =
        std::function<std::shared_ptr<GraphicsPipeline>(GraphicsPipelineBuilder &, DescriptorSetLayoutBuilder &)>;

    /// The callables to create the graphics pipelines used in the rendergraph
    std::vector<std::pair<std::string, GraphicsPipelineCreateFunction>> m_pipeline_create_functions;

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
    //  SHADERS
    // ---------------------------------------------------------------------------------------------------------
    std::vector<std::shared_ptr<Shader>> m_shaders;

    // ---------------------------------------------------------------------------------------------------------
    //  TEXTURES
    // ---------------------------------------------------------------------------------------------------------
    /// TODO: Explain how textures are treated equally here
    std::vector<std::shared_ptr<Texture>> m_textures;

    // TODO: Add @exception to documentation of other methods/code parts!

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic!
    void check_for_cycles();

    /// Create the buffers of every buffer resource in the rendergraph
    void create_buffers();

    /// Descriptor management
    // TODO: better naming? create_descriptors?
    void create_descriptor_sets();

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

    /// Validate rendergraph
    /// @note For rendergraph validation, the passes of the rendergraph must already be created
    void validate_render_graph();

public:
    /// Default constructor
    /// @note device and swapchain are not a const reference because rendergraph needs to modify both
    /// @param device The device wrapper
    /// @param swapchain The swapchain wrapper
    RenderGraph(Device &device, Swapchain &swapchain);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a new graphics pass to the rendergraph
    /// @param name The name of the graphics pass
    /// @param on_pass_create A callable to create the graphics pass using GraphicsPassBuilder
    /// @note Move semantics is used to std::move on_pass_create
    void add_graphics_pass(std::string name, GraphicsPassCreateFunction on_pass_create);

    /// Add a new graphics pipeline to the rendergraph
    /// @param name The graphics pipeline name
    /// @param on_pipeline_create A function to create the graphics pipeline using GraphicsPipelineBuilder
    /// @note Move semantics is used to std::move on_pipeline_create
    void add_graphics_pipeline(std::string name, GraphicsPipelineCreateFunction on_pipeline_create);

    /// Add an index buffer to rendergraph
    /// @note The Vulkan index type is set to ``VK_INDEX_TYPE_UINT32`` by default and it not exposed as parameter
    /// @param name The name of the index buffer
    /// @param on_update The update function of the index buffer
    /// @return A shared pointer to the buffer resource that was created
    [[nodiscard]] std::shared_ptr<Buffer>
    add_index_buffer(std::string name, std::optional<std::function<void()>> on_update = std::nullopt);

    // TODO: Use a SPIR-V library like spirv-cross to deduce shader type from the SPIR-V file automatically!

    /// Load a SPIR-V shader from a file
    /// @param name The internal debug name of the shader (not the file name)
    /// @param shader_stage The shader stage
    /// @param file_name The shader file name
    /// @return A shared pointer to the shader that was loaded from the SPIR-V file
    [[nodiscard]] std::shared_ptr<Shader>
    add_shader(std::string name, VkShaderStageFlagBits shader_stage, std::string file_name);

    /// Add a texture to the rendergraph
    /// @param name The name of the texture (must not be empty)
    /// @param uage The texture usage inside of rendergraph
    /// @param format The image format of the texture
    /// @param on_init The initialization function of the texture (``std::nullopt`` by default)
    /// @param on_update The update function of the texture (``std::nullopt`` by default)
    /// @return A shared pointer to the texture that was created
    [[nodiscard]] std::shared_ptr<Texture> add_texture(std::string name,
                                                       TextureUsage usage,
                                                       VkFormat format,
                                                       std::optional<std::function<void()>> on_init = std::nullopt,
                                                       std::optional<std::function<void()>> on_update = std::nullopt);

    /// Add a uniform buffer to rendergraph
    /// @param name The name of the uniform buffer
    /// @param on_update The update function of the uniform buffer
    /// @return A shared pointer to the buffer resource that was created
    [[nodiscard]] std::shared_ptr<Buffer>
    add_uniform_buffer(std::string name, std::optional<std::function<void()>> on_update = std::nullopt);

    /// Add a vertex buffer to rendergraph
    /// @param name The name of the vertex buffer
    /// @param vertex_attributes The vertex input attribute descriptions
    /// @note You might cleverly noticed that ``VkVertexInputAttributeDescription`` is not required to create a buffer.
    /// Why then is it a parameter here? The vertex input attribute description is stored in the buffer so that when
    /// rendergraph gets compiled and builds the graphics pipelines, it can read ``VkVertexInputAttributeDescription``
    /// from the buffers to create the graphics pipelines.
    /// @param on_update The update function of the vertex buffer
    /// @return A shared pointer to the buffer resource that was created
    [[nodiscard]] std::shared_ptr<Buffer>
    add_vertex_buffer(std::string name,
                      std::vector<VkVertexInputAttributeDescription> vertex_attributes,
                      std::optional<std::function<void()>> on_update = std::nullopt);

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
