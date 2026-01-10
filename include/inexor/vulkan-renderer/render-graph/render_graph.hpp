#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

/// Using declaration
using inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder;
using inexor::vulkan_renderer::wrapper::pipelines::PipelineCache;
using wrapper::DebugLabelColor;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;

class RenderGraph {
private:
    // The device wrapper
    Device &m_device;
    /// The buffers (vertex buffers, index buffers, uniform buffers...)
    std::vector<std::shared_ptr<Buffer>> m_buffers;
    /// The textures (back buffers, depth buffers, textures...)
    std::vector<std::shared_ptr<Texture>> m_textures;
    /// The graphics passes
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;
    /// An instance of the graphics pass builder
    GraphicsPassBuilder m_graphics_pass_builder{};
    /// An instance of the graphics pipeline builder
    GraphicsPipelineBuilder m_graphics_pipeline_builder;
    /// The descriptor set layout builder (a builder pattern for descriptor set layouts)
    DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;
    /// An instance of the descriptor set allocator
    DescriptorSetAllocator m_descriptor_set_allocator;
    /// An instance of the write descriptor set builder
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

    ///
    using OnCreateGraphicsPipeline = std::function<void(GraphicsPipelineBuilder &)>;
    ///
    std::vector<OnCreateGraphicsPipeline> m_graphics_pipeline_create_functions;
    /// The unique wait semaphores of all swapchains used (This means if one swapchain is used mutliple times it's still
    /// only one VkSemaphore in here because collect_swapchain_image_available_semaphores method will fill this vector)
    std::vector<VkSemaphore> m_swapchains_imgs_available;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    RenderGraph(Device &device, const PipelineCache &pipeline_cache);

    /// Add a buffer to the rendergraph
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    /// @return A weak pointer to the buffer resource which was created
    [[nodiscard]] std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type, std::function<void()> on_update);

    /// Add a graphics pass to the rendergraph
    /// @param graphics_pass The graphics pass which was created
    /// @return A weak pointer to the graphics pass which was created
    [[nodiscard]] std::weak_ptr<GraphicsPass> add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass);

    /// Add a graphics pipeline to rendergraph
    /// @param on_create_graphics_pipeline The graphics pipe
    void add_graphics_pipeline(OnCreateGraphicsPipeline on_create_graphics_pipeline);

    void allocate_descriptor_sets();

    /// Add a resource descriptor to the rendergraph
    /// @param on_build_descriptor_set_layout
    /// @param on_allocate_descriptor_set
    /// @param on_update_descriptor_set
    void add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                 OnAllocateDescriptorSet on_allocate_descriptor_set,
                                 OnBuildWriteDescriptorSets on_update_descriptor_set);

    // @TODO How to handle optional texture update depending on texture type?
    // @TODO By implementing textures which are not updated, but only initliazed, we could save memory!

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
                                                     std::uint32_t width, std::uint32_t height, std::uint32_t channels,
                                                     VkSampleCountFlagBits sample_count,
                                                     std::function<void()> on_update);

    void create_descriptor_set_layouts();

    void create_graphics_pipelines();

    void check_for_cycles();

    void collect_swapchain_img_available_semaphores();

    /// Compile the rendergraph
    void compile();

    /// Fill the VkRenderingInfo for a graphics pass
    /// @param pass The graphics pass
    void fill_graphics_pass_rendering_info(GraphicsPass &pass);

    /// @NOTE This get method cannot be const because a builder modifies its data when being used!
    [[nodiscard]] GraphicsPassBuilder &get_graphics_pass_builder() {
        return m_graphics_pass_builder;
    }

    /// @NOTE This get method cannot be const because a builder modifies its data when being used!
    [[nodiscard]] GraphicsPipelineBuilder &get_graphics_pipeline_builder() {
        return m_graphics_pipeline_builder;
    }

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

    /// Render the rendergraph
    void render();

    /// Reset the entire rendergraph
    void reset();

    /// Sort the graphics passes their dependencies
    void sort_graphics_passes_by_order();

    void update_buffers();

    void update_textures();

    void update_write_descriptor_sets();
};

} // namespace inexor::vulkan_renderer::render_graph
