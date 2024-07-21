#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <spdlog/spdlog.h>
#include <volk.h>

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
using wrapper::pipelines::PipelineLayout;

/// A rendergraph is a generic solution for rendering architecture.
///
///
///
///
///
///
class RenderGraph {
private:
    /// The device wrapper
    Device &m_device;

    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    /// -----------------------------------------------------------------------------------------------------------------
    ///  GRAPHICS PASSES
    /// -----------------------------------------------------------------------------------------------------------------
    /// Graphics passes are build inside of graphics pass create functions. Those functions are given to the
    /// rendergraph, and they are all called sequentially during rendergraph compilation. Inside of the graphics pass
    /// create function, the GraphicsPassBuilder can be used to build the graphics pass. The graphics pass which is
    /// created is stored internally inside of the rendergraph. Each graphics pass specifies to which attachments
    /// (color, depth, stencil) it writes by using the writes_to method of the GraphicsPassBuilder. The attachments will
    /// be used by rendergraph in the VkRenderingInfo in dynamic rendering. Each pass must also specify from which other
    /// passes it reads. For example if we specify B.reads_from(A), it means that graphics pass B is reading from
    /// graphics pass A. Rendergraph can then automatically determine the oder of all passes during rendergraph
    /// compilation using depth first search (DFS) algorithm. Rendergraph also makes sure that the graph is acyclic, so
    /// there must not be any cycles in it! Note that graphics passes only specify reads_from for previous passes and
    /// writes_to for attachments, as a graphics pass does not directly write into another graphics pass, but only into
    /// attachments. Why can rendergraph only reason about cycles after creating the graphics passes, at the and of
    /// compilation? To check for cycles, we would need to reason about the reads of each graphics pass before the
    /// passes have been created. The problem here is that we can't reference a piece of memory unless it as been
    /// created, meaning we can't read_from a pass unless its shared_ptr has been allocated. A workaround would be to
    /// have a default constructor which already creates the object before rendergraph could call another constructor
    /// which actually creates the pass. This is however much more complex than the current solution, and from what we
    /// understand it's the fact that we can't reference a piece of memory unless it has been created that makes it very
    /// hard to actually make cycles in rendergraph passes! You can only call reads_from for passes which came before
    /// the pass you are currently creating, you can't call reads_from for passes which come later.
    /// -----------------------------------------------------------------------------------------------------------------

    /// The graphics pass builder of the rendergraph
    GraphicsPassBuilder m_graphics_pass_builder{};
    /// In these callback functions, the graphics passes will be created
    using OnCreateGraphicsPass = std::function<std::shared_ptr<GraphicsPass>(GraphicsPassBuilder &)>;
    /// The graphics pass create functions
    std::vector<OnCreateGraphicsPass> m_graphics_pass_create_functions;
    /// The graphics passes used in the rendergraph
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;

    /// -----------------------------------------------------------------------------------------------------------------
    ///  SWAPCHAINS
    /// -----------------------------------------------------------------------------------------------------------------

    // TODO
    std::vector<VkSemaphore> m_swapchains_imgs_available;

    /// -----------------------------------------------------------------------------------------------------------------
    ///  GRAPHICS PIPELINES
    /// -----------------------------------------------------------------------------------------------------------------
    /// Graphics pipelines are created during rendergraph compilation, but the graphics pipeline instances are not
    /// stored inside of the rendergraph (unlike textures or buffers for example). The reason for this is that graphics
    /// pipelines are also not bound by the rendergraph automatically before calling the on_record command buffer
    /// recording function of a graphics pass. This is because while the rendergraph could bind a graphics pipeline just
    /// before calling on_record, inside of the on_record function, the use of graphics pipelines could be arbitrarily
    /// complex. We also didn't want to have a rendergraph API which exposes binding one graphics pipeline before
    /// calling on_record because this could let the programmer think that binding pipelines is fully covered internally
    /// in rendergraph, which is not the case. You might wonder why we decided to use create functions for the graphics
    /// pipelines if the graphics pipeline is not stored inside of rendergraph? To create a graphics pipeline, you need
    /// a pipeline layout. The pipeline layout is part of the GraphicsPipeline wrapper and every graphics pipeline has
    /// exactly one pipeline layout. To create the pipeline layout, you need to know the descriptor set layout! To be
    /// precise, you need to know the descriptor set layoput if the graphics pipeline uses resource descriptors. You
    /// also need to know the push constant ranges, but they are relatively easy to specify in GraphicsPipelienBuilder.
    /// This complex order of initialization of Vulkan resources must be respected and one of the advantages of having a
    /// rendergraph is that is makes all this very easy. After the descriptor set layout has been created by the
    /// rendergraph, the graphics pipelines can be created because we then know about the descriptor set layout which is
    /// required for the pipeline layout.
    /// -----------------------------------------------------------------------------------------------------------------

    // TODO: Support compute pipelines
    // TODO: Use pipeline cache and write pipeline cache to file, and support loading them

    /// The graphics pipeline builder of the rendergraph
    GraphicsPipelineBuilder m_graphics_pipeline_builder;
    /// In these callback functions, the graphics pipelines will be created
    using OnCreateGraphicsPipeline = std::function<void(GraphicsPipelineBuilder &)>;
    /// The graphics pipeline create functions
    std::vector<OnCreateGraphicsPipeline> m_pipeline_create_functions;

    /// -----------------------------------------------------------------------------------------------------------------
    ///  BUFFERS
    /// -----------------------------------------------------------------------------------------------------------------
    /// We use Vulkan Memory Allocator (VMA) for memory management of resources under the hood. Buffers are created and
    /// stored inside of rendergraph exclusively. To code outside of rendergraph, we give std::weak_ptr of the Buffer
    /// handles we create. This way, the buffers can be used in the on_record function directly without having to pass
    /// the buffers as parameters to the on_record function. Also, the use of std::weak_ptr makes the memory ownership
    /// of the buffer handle clear. The memory is owned by the rendergraph in a std::shared_ptr, but it can be used
    /// outside of rendergraph through the std::weak_ptr. The Buffer wrapper can be a VERTEX_BUFFER, INDEX_BUFFER, or
    /// UNIFORM_BUFFER. The rendergraph can be instructed to update a buffer in code outside of the rendergraph using
    /// the request_update method, and the update is then carried out by rendergraph. All buffer updates are carried out
    /// on a per-frame basis, meaning that all updates will always be coherent with respect to one frame. In the future,
    /// we could improve rendergraph so it automatically double or triple buffers all resources (buffers, textures..),
    /// so it can use one index for rendering, while update on the next index is already being carried out. This will
    /// parallelization using taskflow and proper synchronization must be used. While this would increase memory
    /// consumption, it would improve rendering performance by reducing cpu and gpu stalls. Every buffer must have an
    /// update function. If a vertex buffer and an index buffer is updated, each buffer should be updated in their own
    /// update function rather than updating the index buffer in the vertex buffer as well. While this is technically
    /// also correct, it makes no sense to do it this way because vertex buffer and index buffer are updated on a
    /// per-frame basis coherently anyways. So it doesn't really matter.
    /// -----------------------------------------------------------------------------------------------------------------

    /// The vertex-, index-, and uniform buffers
    std::vector<std::shared_ptr<Buffer>> m_buffers;

    /// -----------------------------------------------------------------------------------------------------------------
    ///  TEXTURES
    /// -----------------------------------------------------------------------------------------------------------------
    /// Like buffers, textures are created by rendergraph and stored in the rendergraph in a shared_ptr. This means just
    /// like buffers, the rendergraph owns the underlying memory, and all external code must use std::weak_ptr to it.
    /// There are different types of textures, which are listed in TextureUsage. In principle, the Texture wrapper
    /// allows each texture to have an on_init and an on_update function. Depending on the texture usage, textures can
    /// be created in two different ways: Textures which are used as color attachment (back buffer) or depth attachment
    /// (depth buffer) are created inside of rendergraph only. This means code outside of rendergraph is not allowed to
    /// modify the data in the texture directly, but instead the external code must specify which attachment is written
    /// to by which pass. Rendergraph will then write to the attachments automatically. This means for texture which are
    /// of TextureUsage COLOR_ATTACHMENT or DEPTH_ATTACHMENT, both on_init and on_update are std::nullopt (Rendergraph
    /// initializes these textures directly rather than specifying an unnecessary on_init function which is then called
    /// directly). The other type of texture is TextureUsage::NORMAL, which means this texture is used in for example in
    /// combined image samplers. This could be the font texture of ImGui for example. Because the data to fill this
    /// texture is loaded from a file by the ImGui wrapper, we must specify an on_init function to rendergraph which
    /// copies the font texture data into the texture wrapper. TextureUsage NORMAL is the only texture type which is
    /// allowed to have an on_update function. The on_update function is called per-frame (if it's not std::nullopt).
    /// The on_update function allows for dynamic textures which are updated every frame. Note that so far, our code
    /// base does not use this feature yet.
    std::vector<std::shared_ptr<Texture>> m_textures;

    /// -----------------------------------------------------------------------------------------------------------------
    ///  RESOURCE DESCRIPTORS
    /// -----------------------------------------------------------------------------------------------------------------
    /// After a lot of discussion we decided to keep the actual VkDescriptorSet handles not inside of of
    /// rendergraph. Originally we planed to have one descriptor set per pass (or several of them per pass) which are
    /// bound using vkBindDescriptorSet before on_record is called. However, the binding of descriptor sets inside of
    /// the on_record command buffer recording function of the pass can be much more complex than just binding one
    /// descriptor set (or several) before calling on_record. In fact, the descriptor set binding could be arbitrarily
    /// complex inside of on_record. Associating the descriptor sets with the pass would introduce another level of
    /// unnecessary indirection without any benefits which we do not want. We then thought we could automatically create
    /// the descriptor set layout of a pass by analyzing the resources the pass reads from. This in theory would also
    /// allow us to allocate the descriptor sets and to update them. However, the descriptor set layout is also required
    /// for creating the pipeline layout! There are two problems with this: 1) The programmer would either have to
    /// specify the reads of the pass in the order of the descriptor set layout or associate the read of a pass with a
    /// binding in the descriptor set layout. Otherwise, the descriptor set layout would be messed up. 2) Because the
    /// descriptor set layout is required when creating the graphics pipeline, we would have to associate pipelines with
    /// passes somehow. However, we also decided to keep pipelines (instances of the GraphicsPipeline wrapper) out of
    /// rendergraph. In summary, keeping VkDescriptorSet handles in rendergraph would complicate the API unnecessarily.
    /// Rendergraph now manages resource descriptors as follows: Descriptors need a descriptor set layout, which is
    /// created from DescriptorSetLayoutBuilder (the first function). Inside of OnBuildDescriptorSetLayout, with the
    /// help of DescriptorSetLayoutBuilder, the programmer simply specifies which descriptors are inside of the
    /// descriptor set. The descriptor set is then created by the builder. DescriptorSetLayoutBuilder uses
    /// DescriptorSetLayoutCache internally to potentially speed up (re)creation of descriptor set layouts. The
    /// VkDescriptorSetLayout created by DescriptorSetLayoutBuilder inside of on_build_descriptor_set_layout must be
    /// stored externally, and it is the responsibility of the programmer to make sure the VkDescriptorSetLayout is
    /// valid memory when it is used in the on_record function! The descriptor sets are allocated by rendergraph via
    /// OnAllocateDescriptorSet functions using the DescriptorSetAllocator of the rendergraph. Descriptor sets are
    /// updated in the OnUpdateDescriptorSet functions using the DescriptorSetUpdateBuilder of the rendergraph.
    ///
    /// TODO: Mention batching to vkUpdateDescriptorSets...
    /// -----------------------------------------------------------------------------------------------------------------

    /// The descriptor set layout builder (a builder pattern for descriptor set layouts)
    DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;
    /// The descriptor set allocator
    DescriptorSetAllocator m_descriptor_set_allocator;
    /// The write descriptor set builder (a builder pattern for write descriptor sets)
    WriteDescriptorSetBuilder m_write_descriptor_set_builder;

    /// A user-defined function which creates the descriptor set layout
    using OnBuildDescriptorSetLayout = std::function<void(DescriptorSetLayoutBuilder &)>;
    /// A user-defined function which allocates a descriptor set
    using OnAllocateDescriptorSet = std::function<void(DescriptorSetAllocator &)>;
    /// A user-defined function which builds the descriptor set write for the pass
    using OnBuildWriteDescriptorSets = std::function<std::vector<VkWriteDescriptorSet>(WriteDescriptorSetBuilder &)>;

    /// Resource descriptors are managed by specifying those three functions to the rendergraph
    /// Rendergraph will then call those function in the correct order during rendergraph compilation
    using ResourceDescriptor =
        std::tuple<OnBuildDescriptorSetLayout, OnAllocateDescriptorSet, OnBuildWriteDescriptorSets>;
    /// The resource descriptors of the rendergraph
    std::vector<ResourceDescriptor> m_resource_descriptors;
    /// All write descriptor sets will be stored in here so we can have one batched call to vkUpdateDescriptorSets
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;

    /// Allocate the descriptor sets
    void allocate_descriptor_sets();

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic!
    void check_for_cycles();

    /// Collect all image available semaphores of all swapchains which are used into one std::vector<VkSemaphore>
    void collect_swapchain_image_available_semaphores();

    /// Create the descriptor set layouts
    void create_descriptor_set_layouts();

    /// Create the graphics passes
    void create_graphics_passes();

    /// Create the graphics pipelines
    void create_graphics_pipelines();

    /// Determine the order of execution of the graphics passes by using depth first search (DFS) algorithm
    void determine_pass_order();

    /// Fill the VkRenderingInfo for a graphics pass
    /// @param pass The graphics pass
    void fill_graphics_pass_rendering_info(GraphicsPass &pass);

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
    void record_command_buffer_for_pass(const CommandBuffer &cmd_buf, GraphicsPass &pass);

    /// Update the vertex-, index-, and uniform-buffers
    /// @note If a uniform buffer has been updated, an update of the associated descriptor set will be performed
    void update_buffers();

    /// Update dynamic textures
    void update_textures();

    /// Update the write descriptor sets
    /// @note This function must only be called once during rendergraph compilation, not for every frame!
    void update_write_descriptor_sets();

    /// Make sure all required resources are specified so rendergraph is ready to be compiled
    void validate_render_graph();

public:
    /// Default constructor
    /// @note device and swapchain are not taken as a const reference because rendergraph needs to modify both
    /// @param device The device wrapper
    RenderGraph(Device &device);

    RenderGraph(const RenderGraph &) = delete;
    // TODO: Implement me!
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a new graphics pass to the rendergraph
    /// @param on_pass_create A callable to create the graphics pass using GraphicsPassBuilder
    /// @note Move semantics is used to std::move on_pass_create
    void add_graphics_pass(OnCreateGraphicsPass on_pass_create);

    /// Add a new graphics pipeline to the rendergraph
    /// @param on_pipeline_create A function to create the graphics pipeline using GraphicsPipelineBuilder
    /// @note Move semantics is used to std::move on_pipeline_create
    void add_graphics_pipeline(OnCreateGraphicsPipeline on_pipeline_create);

    /// Add an buffer to rendergraph
    /// @param name The name of the buffer
    /// @param buffer_type The type of the buffer
    /// @param on_update The update function of the index buffer
    /// @return A weak pointer to the buffer resource that was created
    [[nodiscard]] std::weak_ptr<Buffer>
    add_buffer(std::string buffer_name, BufferType buffer_type, std::function<void()> on_update);

    /// Add a descriptor to rendergraph
    /// @note This function is of type void because it does not store anything that is created in those callback
    /// functions. As mentioned above, resource descriptors are kept outside of rendergraph.
    /// @note This function does not perform any error checks when it comes to correct usage of descriptors, because
    /// this is the job of validation layers. If you would give the rendergraph 3 empty functions, you would not notice
    /// unless you attempt to use some descriptor in on_record that would not have been set up correctly.
    /// @param on_create_descriptor_set_layout The descriptor set layout build function
    /// @param on_allocate_descriptor_set The descriptor set allocation function
    /// @param on_update_descriptor_set The descriptor set update function
    void add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                 OnAllocateDescriptorSet on_allocate_descriptor_set,
                                 OnBuildWriteDescriptorSets on_update_descriptor_set);

    /// Add a texture which will be initialized externally (not inside of rendergraph)
    /// @param texture_name The name of the texture
    /// @param usage The usage of the texture inside of rendergraph
    /// @param format The format of the texture
    /// @param sample_count The sample count of the texture
    /// @param m_on_check_for_updates The texture update function (an empty lambda by default)
    /// @return A weak pointer to the texture that was created
    [[nodiscard]] std::weak_ptr<Texture> add_texture(
        std::string texture_name,
        TextureUsage usage,
        VkFormat format,
        std::uint32_t width,
        std::uint32_t height,
        VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
        std::function<void()> m_on_check_for_updates = []() {});

    /// Compile the rendergraph
    void compile();

    /// Render a frame
    void render();

    /// Reset the entire RenderGraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
