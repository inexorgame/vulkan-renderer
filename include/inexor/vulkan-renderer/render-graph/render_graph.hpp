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
    const Device &m_device;
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

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    RenderGraph(const Device &device, const PipelineCache &pipeline_cache);

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

    /// Add a resource descriptor to the rendergraph
    /// @param on_build_descriptor_set_layout
    /// @param on_allocate_descriptor_set
    /// @param on_update_descriptor_set
    [[nodiscard]] void add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
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

    /// Compile the rendergraph
    void compile();

    /// @NOTE This get method cannot be const because a builder modifies its data when being used!
    [[nodiscard]] GraphicsPassBuilder &get_graphics_pass_builder() {
        return m_graphics_pass_builder;
    }

    /// @NOTE This get method cannot be const because a builder modifies its data when being used!
    [[nodiscard]] GraphicsPipelineBuilder &get_graphics_pipeline_builder() {
        return m_graphics_pipeline_builder;
    }

    /// Render the rendergraph
    void render();

    /// Reset the entire rendergraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
