#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"

#include <memory>
#include <string>
#include <vector>

// Forward declarations
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::pipelines {

/// Builder class for VkPipelineCreateInfo
/// @note that all members are initialized in the method ``reset()`` (This method is also called by the constructor)
/// Calling ``reset()`` allows you to re-use this builder for the next graphics pipeline you want to build, so you don't
/// need to create one builder per graphics pipeline you build
class GraphicsPipelineBuilder {
private:
    const Device &m_device;

    std::vector<VkVertexInputBindingDescription> m_vertex_input_binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> m_vertex_input_attribute_descriptions;
    // With the builder we can fill vertex binding descriptions and vertex attribute descriptions in here
    VkPipelineVertexInputStateCreateInfo m_vertex_input_sci;

    // With the builder we can set topology in here
    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_sci;

    // With the builder we can set the patch control point count in here
    VkPipelineTessellationStateCreateInfo m_tesselation_sci;

    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    // With the builder we can set viewport(s) and scissor(s) in here
    VkPipelineViewportStateCreateInfo m_viewport_sci;

    // With the builder we can set polygon mode, cull mode, front face, and line width
    // TODO: Implement methods to enable depth bias and for setting the depth bias parameters
    VkPipelineRasterizationStateCreateInfo m_rasterization_sci;

    // With the builder we can set rasterization samples and min sample shading
    // TODO: Expose more multisampling parameters if desired
    VkPipelineMultisampleStateCreateInfo m_multisample_sci;

    // With the builder we can't set individial fields of this struct,
    // since it's easier to specify an entire VkPipelineDepthStencilStateCreateInfo struct to the builder instead
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_sci;

    // With the builder we can't set individial fields of this struct,
    // since it's easier to specify an entire VkPipelineColorBlendStateCreateInfo struct to the builder instead
    VkPipelineColorBlendStateCreateInfo m_color_blend_sci;

    std::vector<VkDynamicState> m_dynamic_states;
    // This will be filled in the build() method
    // With the builder we can specify a std::vector<VkDynamicState>
    VkPipelineDynamicStateCreateInfo m_dynamic_states_sci;

    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

    // With the builder we can either call add_shader or set_shaders
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    // With the builder we can either call add_color_blend_attachment or set_color_blend_attachments
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit GraphicsPipelineBuilder(const Device &device);
    GraphicsPipelineBuilder(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept;
    ~GraphicsPipelineBuilder() = default;

    GraphicsPipelineBuilder &operator=(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder &operator=(GraphicsPipelineBuilder &&) = delete;

    /// Add a shader stage
    /// @param shader The shader stage to add
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &add_shader(const VkPipelineShaderStageCreateInfo &shader);

    /// Add a vertex input attribute description
    /// @param description The vertex input attribute description
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    add_vertex_input_attribute(const VkVertexInputAttributeDescription &description);

    /// Add a vertex input binding description
    /// @param description The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &add_vertex_input_binding(const VkVertexInputBindingDescription &description);

    /// Add a color blend attachment
    /// @param attachment The color blend attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment);

    /// Build the graphics pipeline with specified pipeline create flags
    /// @param name The debug name of the graphics pipeline
    /// @return The unique pointer instance of ``GraphicsPipeline`` that was created
    [[nodiscard]] std::unique_ptr<GraphicsPipeline> build(std::string name);

    /// Reset all data in this class so the builder can be re-used
    /// @note This is called by the constructor
    void reset();

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

    /// Set the depth stencil
    /// @param depth_stencil The depth stencil
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil);

    /// Set the dynamic states
    /// @param dynamic_states The dynamic states
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states);

    /// Set the input assembly state create info
    /// @note If you just want to set the triangle topology, call ``set_triangle_topology`` instead, because this is the
    /// most powerful method of this method in case you really need to overwrite it
    /// @param input_assembly The pipeline input state create info
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly);

    /// Set the line width of rasterization
    /// @param line_width The line width of rasterization
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_line_width(float width);

    /// Set the most important MSAA settings
    /// @param sample_count The number of samples used in rasterization
    /// @param min_sample_shading A minimum fraction of sample shading
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_multisampling(VkSampleCountFlagBits sample_count,
                                                             float min_sample_shading);

    /// Store the pipeline layout
    /// @param layout The pipeline layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_pipeline_layout(VkPipelineLayout layout);

    /// Set the triangle topology
    /// @param topology the primitive topology
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_primitive_topology(VkPrimitiveTopology topology);

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

    /// Set the viewport data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissor The scissor in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_scissors(const std::vector<VkRect2D> &scissors);

    /// Set the shader stage
    /// @param shader_stages The shader stages
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shaders);

    /// Set the tesselation control point count
    /// @param control_point_count The tesselation control point count
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_tesselation_control_point_count(std::uint32_t control_point_count);

    /// Set the vertex input attribute descriptions manually
    /// You should prefer to use ``add_vertex_input_attribute`` instead
    /// @param descriptions The vertex input attribute descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &
    set_vertex_input_attributes(const std::vector<VkVertexInputAttributeDescription> &descriptions);

    /// Set the vertex input binding descriptions manually
    /// You should prefer to use ``add_vertex_input_binding`` instead
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

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// @param viewports The viewports in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_viewports(const std::vector<VkViewport> &viewports);

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_wireframe(VkBool32 wireframe);
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
