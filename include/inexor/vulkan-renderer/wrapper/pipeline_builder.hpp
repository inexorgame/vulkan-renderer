#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline.hpp"

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declarations
class Device;

/// Builder class for VkPipelineCreateInfo
class GraphicsPipelineBuilder {
private:
    const Device &m_device;
    VkPipelineVertexInputStateCreateInfo m_vertex_input_sci{make_info<VkPipelineVertexInputStateCreateInfo>()};

    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_sci{make_info<VkPipelineInputAssemblyStateCreateInfo>({
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    })};

    VkPipelineTessellationStateCreateInfo m_tesselation_sci{make_info<VkPipelineTessellationStateCreateInfo>()};
    VkPipelineViewportStateCreateInfo m_viewport_sci{make_info<VkPipelineViewportStateCreateInfo>()};

    VkPipelineRasterizationStateCreateInfo m_rasterization_sci{make_info<VkPipelineRasterizationStateCreateInfo>({
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
    })};

    // TODO: Support more options for multisampling
    VkPipelineMultisampleStateCreateInfo m_multisample_sci{make_info<VkPipelineMultisampleStateCreateInfo>({
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
    })};

    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_sci{make_info<VkPipelineDepthStencilStateCreateInfo>()};
    VkPipelineColorBlendStateCreateInfo m_color_blend_sci{make_info<VkPipelineColorBlendStateCreateInfo>()};
    VkPipelineDynamicStateCreateInfo m_dynamic_states_sci{make_info<VkPipelineDynamicStateCreateInfo>()};

    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};
    VkRenderPass m_render_pass{VK_NULL_HANDLE};

    std::vector<VkDynamicState> m_dynamic_states;
    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    std::vector<VkVertexInputBindingDescription> m_vertex_input_binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> m_vertex_input_attribute_descriptions;
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;

public:
    /// Default constructor
    /// @param device The device wrapper
    GraphicsPipelineBuilder(const Device &device);
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

    /// Set the render pass
    /// @param render_pass The render pass
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_render_pass(VkRenderPass render_pass);

    /// Set the scissor data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissors The scissors in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_scissor(const VkRect2D &scissor);

    /// Set the viewport data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissor The scissor in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_scissors(const std::vector<VkRect2D> &scissors);

    /// Set the shader stage
    /// @param shader_stages The shader stages
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shaders);

    /// Set the tesselation state create info
    /// @param control_points The tesselation control point count
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_tesselation(std::uint32_t control_points);

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

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// @param viewports The viewports in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_viewports(const std::vector<VkViewport> &viewports);

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_wireframe(VkBool32 wireframe);
};

} // namespace inexor::vulkan_renderer::wrapper
