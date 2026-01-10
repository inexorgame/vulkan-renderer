#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Device;
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Forward declaration
class PipelineCache;

// TODO: ComputePipelineBuilder

/// Builder class for VkPipelineCreateInfo for graphics pipelines which use dynamic rendering
/// @note This builder pattern does not perform any checks which are already covered by validation layers.
/// This means if you forget to specify viewport for example, creation of the graphics pipeline will fail.
/// It is the reponsibility of the programmer to use validation layers to check for problems.
/// @TODO Although we initially did not want to implement checks which mimic the validation layers, it might be worth it
/// to implement checks in case required fields are not set in this builder.
class GraphicsPipelineBuilder {
private:
    /// The device wrapper
    const Device &m_device;
    /// The Vulkan pipeline cache
    const PipelineCache &m_pipeline_cache;
    /// The graphics pipeline setup data which will be std::moved into GraphicsPipeline wrapper
    GraphicsPipelineSetupData m_graphics_pipeline_setup_data;

    /// Reset all data in this class so the builder can be re-used
    void reset();

public:
    // TODO: Make default constructor private, so only RenderGraph can access it!

    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    GraphicsPipelineBuilder(const Device &device, const PipelineCache &pipeline_cache);

    GraphicsPipelineBuilder(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept;

    ~GraphicsPipelineBuilder() = default;

    GraphicsPipelineBuilder &operator=(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder &operator=(GraphicsPipelineBuilder &&) = delete;

    /// Adds a color attachment
    /// @param format The format of the color attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &add_color_attachment_format(VkFormat format);

    /// Add a color blend attachment
    /// @param attachment The color blend attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment);

    /// Add the default color blend attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &add_default_color_blend_attachment();

    /// Add a push constant range to the graphics pass
    /// @param shader_stage The shader stage for the push constant range
    /// @param size The size of the push constant
    /// @param offset The offset in the push constant range
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &add_push_constant_range(VkShaderStageFlags shader_stage, std::uint32_t size,
                                                                   std::uint32_t offset = 0);

    ///
    /// @param shader
    /// @return
    [[nodiscard]] GraphicsPipelineBuilder &add_shader(std::weak_ptr<Shader> shader);

    /// Build the graphics pipeline with specified pipeline create flags
    /// @param name The debug name of the graphics pipeline
    /// @TODO Remove this and use only dynamic rendering!
    /// @param use_dynamic_rendering
    /// @return The unique pointer instance of ``GraphicsPipeline`` that was created
    [[nodiscard]] std::shared_ptr<GraphicsPipeline> build(std::string name, bool use_dynamic_rendering);

    /// Set the color blend manually
    /// @param color_blend The color blend
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend);

    /// Set all color blend attachments manually
    /// @note You should prefer to use ``add_color_blend_attachment`` instead
    /// @param attachments The color blend attachments
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_color_blend_attachments(const std::vector<VkPipelineColorBlendAttachmentState> &attachments);

    /// Enable or disable culling
    /// @warning Disabling culling will have a significant performance impact
    /// @param culling_enabled ``true`` if culling is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_culling_mode(VkBool32 culling_enabled);

    /// Set the deptch attachment format
    /// @param format The format of the depth attachment
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_depth_attachment_format(VkFormat format);

    /// Set the descriptor set layout
    /// @param descriptor_set_layouts The descriptor set layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_descriptor_set_layout(VkDescriptorSetLayout descriptor_set_layout);

    /// Set the descriptor set layouts
    /// @param descriptor_set_layouts The descriptor set layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_descriptor_set_layouts(std::vector<VkDescriptorSetLayout> descriptor_set_layouts);

    /// Set the depth stencil
    /// @warning Disabling culling can have performance impacts!
    /// @param depth_stencil The depth stencil
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil);

    /// Set the dynamic states
    /// @param dynamic_states The dynamic states
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states);

    /// Set the stencil attachment format
    /// @param format The format of the stencil attachment
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_stencil_attachment_format(VkFormat format);

    /// Set the input assembly state create info
    /// @note If you just want to set the triangle topology, call ``set_triangle_topology`` instead, because this is the
    /// most powerful method of this method in case you really need to overwrite it
    /// @param input_assembly The pipeline input state create info
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly);

    /// Set the line width of rasterization
    /// @param line_width The line width used in rasterization
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_line_width(float width);

    /// Set the most important MSAA settings
    /// @param sample_count The number of samples used in rasterization
    /// @param min_sample_shading A minimum fraction of sample shading
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_multisampling(VkSampleCountFlagBits sample_count,
                                                             std::optional<float> min_sample_shading = std::nullopt);

    /// Store the pipeline layout
    /// @param layout The pipeline layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_pipeline_layout(VkPipelineLayout layout);

    /// Set the triangle topology
    /// @param topology the primitive topology
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_primitive_topology(VkPrimitiveTopology topology);

    /// Set the renderpass
    /// @param render_pass The renderpass
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_render_pass(const VkRenderPass &render_pass);

    /// Set the push constant ranges
    /// @param push_constant_ranges The push constant ranges
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_push_constant_ranges(std::vector<VkPushConstantRange> push_constant_ranges);

    /// Set the rasterization state of the graphics pipeline manually
    /// @param rasterization The rasterization state
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization);

    /// Set the scissor data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissors The scissors in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_scissor(const VkRect2D &scissor);

    /// Set the scissor data in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkRect2D)
    /// @param extent The extent of the scissor
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_scissor(const VkExtent2D &extent);

    /// Set the shader modules
    /// @param shaders The shader stage create infos
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_shaders(std::vector<VkPipelineShaderStageCreateInfo> shaders);

    /// Set the tesselation control point count
    /// @note This is not used in the code so far, because we are not using tesselation
    /// @param control_point_count The tesselation control point count
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_tesselation_control_point_count(std::uint32_t control_point_count);

    /// Set the vertex input attribute descriptions manually
    /// @note As of C++23, there is no mechanism to do so called reflection in C++, meaning we can't get any information
    /// about the members of a struct, which would allow us to determine vertex input attributes automatically.
    /// @param descriptions The vertex input attribute descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_vertex_input_attributes(const std::vector<VkVertexInputAttributeDescription> &descriptions);

    /// Set the vertex input binding descriptions manually
    /// @param descriptions The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &descriptions);

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// There is another method called set_viewports in case multiple viewports will be used
    /// @param viewport The viewport in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_viewport(const VkViewport &viewport);

    /// Set the viewport in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkViewport)
    /// @param extent The extent of the viewport
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_viewport(const VkExtent2D &extent);

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_wireframe(VkBool32 wireframe);
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
