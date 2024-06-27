#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/render-graph/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Forward declarations
namespace inexor::vulkan_renderer::wrapper {
class Device;
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
class RenderGraph;
}

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: ComputePipelineBuilder

/// Builder class for VkPipelineCreateInfo for graphics pipelines which use dynamic rendering
/// @note This builder pattern does not perform any checks which are already covered by validation layers.
/// This means if you forget to specify viewport for example, creation of the graphics pipeline will fail.
/// It is the reponsibility of the programmer to use validation layers to check for problems.
class GraphicsPipelineBuilder {
private:
    friend class render_graph::RenderGraph;

    /// The device wrapper reference
    const Device &m_device;

    // We are not using member initializers here:
    // Note that all members are initialized in the reset() method
    // This method is also called after the graphics pipeline has been created,
    // allowing one instance of GraphicsPipelineBuilder to be reused

    /// This is used for dynamic rendering
    VkFormat m_swapchain_img_format;
    VkFormat m_depth_attachment_format;
    VkFormat m_stencil_attachment_format;
    VkPipelineRenderingCreateInfo m_pipeline_rendering_ci;

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
    VkPipelineDynamicStateCreateInfo m_dynamic_states_sci;

    /// The layout of the graphics pipeline
    VkPipelineLayout m_pipeline_layout;

    // With the builder we can either call add_shader or set_shaders
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

    // With the builder we can either call add_color_blend_attachment or set_color_blend_attachments
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;

    /// Reset all data in this class so the builder can be re-used
    /// @note This is called by the constructor
    void reset();

    /// Default constructor is private, so only rendergraph can access it
    /// @param device The device wrapper
    explicit GraphicsPipelineBuilder(const Device &device);

public:
    GraphicsPipelineBuilder(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept;
    ~GraphicsPipelineBuilder() = default;

    GraphicsPipelineBuilder &operator=(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder &operator=(GraphicsPipelineBuilder &&) = delete;

    /// Add a shader to the graphics pipeline
    /// @param shader The shader
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &uses_shader(std::shared_ptr<render_graph::Shader> shader) {
        m_shader_stages.push_back(make_info<VkPipelineShaderStageCreateInfo>({
            .stage = shader->m_shader_stage,
            .module = shader->m_shader_module,
            .pName = "main",

        }));
        return *this;
    }

    /// Add a color blend attachment
    /// @param attachment The color blend attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
        m_color_blend_attachment_states.push_back(attachment);
        return *this;
    }

    /// Build the graphics pipeline with specified pipeline create flags
    /// @param name The debug name of the graphics pipeline
    /// @return The unique pointer instance of ``GraphicsPipeline`` that was created
    [[nodiscard]] std::shared_ptr<GraphicsPipeline> build(std::string name);

    /// Set the color blend manually
    /// @param color_blend The color blend
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend) {
        m_color_blend_sci = color_blend;
        return *this;
    }

    /// Set all color blend attachments manually
    /// @note You should prefer to use ``add_color_blend_attachment`` instead
    /// @param attachments The color blend attachments
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &
    set_color_blend_attachments(const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
        m_color_blend_attachment_states = attachments;
        return *this;
    }

    /// Enable or disable culling
    /// @warning Disabling culling will have a significant performance impact
    /// @param culling_enabled ``true`` if culling is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_culling_mode(const VkBool32 culling_enabled) {
        if (culling_enabled == VK_FALSE) {
            spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
        }
        m_rasterization_sci.cullMode = culling_enabled == VK_TRUE ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
        return *this;
    }

    /// Set the depth stencil
    /// @warning Disabling culling can have performance impacts!
    /// @param depth_stencil The depth stencil
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
        m_depth_stencil_sci = depth_stencil;
        return *this;
    }

    /// Set the dynamic states
    /// @param dynamic_states The dynamic states
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states) {
        assert(!dynamic_states.empty());
        m_dynamic_states = dynamic_states;
        return *this;
    }

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
    [[nodiscard]] auto &set_line_width(const float width) {
        m_rasterization_sci.lineWidth = width;
        return *this;
    }

    /// Set the most important MSAA settings
    /// @param sample_count The number of samples used in rasterization
    /// @param min_sample_shading A minimum fraction of sample shading
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_multisampling(const VkSampleCountFlagBits sample_count,
                                          const std::optional<float> min_sample_shading) {
        m_multisample_sci.rasterizationSamples = sample_count;

        if (min_sample_shading) {
            m_multisample_sci.minSampleShading = min_sample_shading.value();
        }
        return *this;
    }

    /// Store the pipeline layout
    /// @param layout The pipeline layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_pipeline_layout(const VkPipelineLayout layout) {
        assert(layout);
        m_pipeline_layout = layout;
        return *this;
    }

    /// Set the triangle topology
    /// @param topology the primitive topology
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_primitive_topology(const VkPrimitiveTopology topology) {
        m_input_assembly_sci.topology = topology;
        return *this;
    }

    /// Set the rasterization state of the graphics pipeline manually
    /// @param rasterization The rasterization state
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
        m_rasterization_sci = rasterization;
        return *this;
    }

    /// Set the scissor data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissors The scissors in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_scissor(const VkRect2D &scissor) {
        m_scissors = {scissor};
        m_viewport_sci.scissorCount = 1;
        m_viewport_sci.pScissors = m_scissors.data();
        return *this;
    }

    /// Set the scissor data in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkRect2D)
    /// @param extent The extent of the scissor
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_scissor(const VkExtent2D &extent) {
        return set_scissor({
            // Convert VkExtent2D to VkRect2D
            .extent = extent,
        });
    }

    /// Set the viewport data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissor The scissor in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_scissors(const std::vector<VkRect2D> &scissors) {
        assert(!scissors.empty());
        m_scissors = scissors;
        m_viewport_sci.scissorCount = static_cast<std::uint32_t>(scissors.size());
        m_viewport_sci.pScissors = scissors.data();
        return *this;
    }

    /// Set the shader stage
    /// @param shader_stages The shader stages
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsPipelineBuilder &set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shaders);

    /// Set the tesselation control point count
    /// @note This is not used in the code so far, because we are not using tesselation
    /// @param control_point_count The tesselation control point count
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_tesselation_control_point_count(const std::uint32_t control_point_count) {
        m_tesselation_sci.patchControlPoints = control_point_count;
        return *this;
    }

    /// Set the vertex input attribute descriptions manually
    /// @note As of C++23, there is no mechanism to do so called reflection in C++, meaning we can't get any information
    /// about the members of a struct, which would allow us to determine vertex input attributes automatically
    /// @param descriptions The vertex input attribute descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &
    set_vertex_input_attributes(const std::vector<VkVertexInputAttributeDescription> &descriptions) {
        assert(!descriptions.empty());
        m_vertex_input_attribute_descriptions = descriptions;
        return *this;
    }

    /// Set the vertex input binding descriptions manually
    /// @param descriptions The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &descriptions) {
        assert(!descriptions.empty());
        m_vertex_input_binding_descriptions = descriptions;
        return *this;
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// There is another method called set_viewports in case multiple viewports will be used
    /// @param viewport The viewport in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_viewport(const VkViewport &viewport) {
        m_viewports = {viewport};
        m_viewport_sci.viewportCount = 1;
        m_viewport_sci.pViewports = m_viewports.data();
        return *this;
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkViewport)
    /// @param extent The extent of the viewport
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_viewport(const VkExtent2D &extent) {
        return set_viewport({
            // Convert VkExtent2D to VkViewport
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .maxDepth = 1.0f,
        });
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// @param viewports The viewports in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_viewports(const std::vector<VkViewport> &viewports) {
        assert(!viewports.empty());
        m_viewports = viewports;
        m_viewport_sci.viewportCount = static_cast<std::uint32_t>(m_viewports.size());
        m_viewport_sci.pViewports = m_viewports.data();
        return *this;
    }

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] auto &set_wireframe(const VkBool32 wireframe) {
        m_rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
