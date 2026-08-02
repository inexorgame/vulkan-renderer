#pragma once

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/shaders/shader.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declarations
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::shaders {
// Forward declarations
class Shader;
} // namespace inexor::vulkan_renderer::wrapper::shaders

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Using declaration
using shaders::Shader;
using tools::InexorException;

// TODO: ComputePipelineBuilder

/// Builder class for VkPipelineCreateInfo for graphics pipelines which use dynamic rendering
/// @note This builder pattern does not perform any checks which are already covered by validation layers.
/// This means if you forget to specify viewport for example, creation of the graphics pipeline will fail.
/// It is the reponsibility of the programmer to use validation layers to check for problems.
/// @TODO Although we initially did not want to implement checks which mimic the validation layers, it might be worth it
/// to implement checks in case required fields are not set in this builder.
class GraphicsPipelineBuilder {
private:
    const core::Device &m_device;
    GraphicsPipelineSetupData m_data;
    void reset();

public:
    // TODO: Make default constructor private, so only RenderGraph can access it!

    /// Default constructor
    /// @param device The device wrapper
    GraphicsPipelineBuilder(const core::Device &device);

    /// Adds a color attachment
    /// @param format The format of the color attachment
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &add_color_attachment_format(const VkFormat format) {
        // @TODO How does this relate to add_color_blend_attachment?
        m_data.color_attachments.push_back(format);
        return *this;
    }

    /// Add a color blend attachment
    /// @param attachment The color blend attachment
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
        m_data.color_blend_attachment_states.push_back(attachment);
        return *this;
    }

    /// Add the standard alpha blend attachment
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] GraphicsPipelineBuilder &add_standard_alpha_blend_attachment();

    /// Add a push constant range to the graphics pass
    /// @param shader_stage The shader stage for the push constant range
    /// @param size The size of the push constant
    /// @param offset The offset in the push constant range (``0`` by default)
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &add_push_constant_range(const VkShaderStageFlags shader_stage, const std::uint32_t size,
                                                const std::uint32_t offset = 0) {
        m_data.push_constant_ranges.emplace_back(VkPushConstantRange{
            .stageFlags = shader_stage,
            .offset = offset,
            .size = size,
        });
        return *this;
    }

    /// Add a shader to the graphics pipeline
    /// @param shader The shader to add
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &add_shader(std::weak_ptr<Shader> shader) {
        if (shader.expired()) {
            throw InexorException("Error: Parameter 'shader' is invalid!");
        }
        m_data.shader_stages.emplace_back(tools::make_info<VkPipelineShaderStageCreateInfo>({
            .stage = shader.lock()->shader_stage(),
            .module = shader.lock()->shader_module(),
            .pName = shader.lock()->entry_point().c_str(),
        }));
        return *this;
    }

    /// Build the graphics pipeline with specified pipeline create flags
    /// @param name The debug name of the graphics pipeline
    /// @TODO Remove this and use only dynamic rendering!
    /// @param use_dynamic_rendering
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] std::shared_ptr<GraphicsPipeline> build(std::string name);

    /// Set the color blend manually
    /// @param color_blend The color blend
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend) {
        m_data.color_blend_sci = color_blend;
        return *this;
    }

    /// Set all color blend attachments manually
    /// @note You should prefer to use ``add_color_blend_attachment`` instead
    /// @param attachments The color blend attachments
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &
    set_color_blend_attachments(const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
        m_data.color_blend_attachment_states = attachments;
        return *this;
    }

    /// Enable or disable culling
    /// @warning Disabling culling will have a significant performance impact
    /// @param culling_enabled ``true`` if culling is enabled
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_culling_mode(const VkBool32 culling_enabled) {
        if (culling_enabled == VK_FALSE) {
            spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
        }
        m_data.rasterization_sci.cullMode = culling_enabled == VK_TRUE ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
        return *this;
    }

    /// Set the deptch attachment format
    /// @param format The format of the depth attachment
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_depth_attachment_format(const VkFormat format) {
        m_data.depth_attachment_format = format;
        return *this;
    }

    /// Set the descriptor set layout
    /// @param descriptor_set_layouts The descriptor set layout
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_descriptor_set_layout(const VkDescriptorSetLayout descriptor_set_layout) {
        if (!descriptor_set_layout) {
            throw InexorException("Error: Parameter 'descriptor_set_layout' is invalid!");
        }
        m_data.descriptor_set_layouts = {descriptor_set_layout};
        return *this;
    }

    /// Set the descriptor set layouts
    /// @param descriptor_set_layouts The descriptor set layout
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_descriptor_set_layouts(std::vector<VkDescriptorSetLayout> descriptor_set_layouts) {
        if (descriptor_set_layouts.empty()) {
            throw InexorException("Error: Parameter 'descriptor_set_layouts' is empty!");
        }
        m_data.descriptor_set_layouts = std::move(descriptor_set_layouts);
        return *this;
    }

    /// Associate a descriptor set resource with the graphics pipeline.
    /// The pipeline layout will be linked to this descriptor set automatically after build.
    [[nodiscard]] auto &add_descriptor_set(std::weak_ptr<wrapper::descriptors::PerFrameDescriptorSets> descriptor_set) {
        if (descriptor_set.expired()) {
            throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
        }
        m_data.associated_descriptor_sets.emplace_back(std::move(descriptor_set));
        return *this;
    }

    /// Set the depth stencil
    /// @warning Disabling culling can have performance impacts!
    /// @param depth_stencil The depth stencil
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
        m_data.depth_stencil_sci = depth_stencil;
        return *this;
    }

    /// Set the standard depth stencil state
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] GraphicsPipelineBuilder &set_standard_depth_stencil();

    /// Set scissor state as dynamic
    /// @todo Implement multiple dynamic scissors with VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_dynamic_scissor() {
        m_data.dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
        m_data.scissors = {};
        m_data.viewport_sci.scissorCount = 1;
        m_data.viewport_sci.pScissors = nullptr;
        return *this;
    }

    /// Set viewport state as dynamic
    /// @todo Implement multiple dynamic scissors with VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_dynamic_viewport() {
        m_data.dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        m_data.viewports = {};
        m_data.viewport_sci.viewportCount = 1;
        m_data.viewport_sci.pViewports = nullptr;
        return *this;
    }

    /// Set the stencil attachment format
    /// @param format The format of the stencil attachment
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_stencil_attachment_format(const VkFormat format) {
        m_data.stencil_attachment_format = format;
        return *this;
    }

    /// Set the input assembly state create info
    /// @note If you just want to set the triangle topology, call ``set_triangle_topology`` instead, because this is the
    /// most powerful method of this method in case you really need to overwrite it
    /// @param input_assembly The pipeline input state create info
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly) {
        m_data.input_assembly_sci = input_assembly;
        return *this;
    }

    /// Set the line width of rasterization
    /// @param line_width The line width used in rasterization
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_line_width(const float width) {
        m_data.rasterization_sci.lineWidth = width;
        return *this;
    }

    /// Set the most important MSAA settings
    /// @param sample_count The number of samples used in rasterization
    /// @param min_sample_shading A minimum fraction of sample shading (``std::nullopt`` by default)
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_multisampling(const VkSampleCountFlagBits sample_count,
                                          const std::optional<float> min_sample_shading = std::nullopt) {
        m_data.multisample_sci.rasterizationSamples = sample_count;
        if (min_sample_shading) {
            m_data.multisample_sci.minSampleShading = min_sample_shading.value();
        }
        return *this;
    }

    /// Store the pipeline layout
    /// @param layout The pipeline layout
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_pipeline_layout(const VkPipelineLayout layout) {
        if (layout) {
            throw InexorException("Error: Parameter 'layout' is invalid!");
        }
        m_data.pipeline_layout = layout;
        return *this;
    }

    /// Set the triangle topology
    /// @param topology the primitive topology
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_primitive_topology(const VkPrimitiveTopology topology) {
        m_data.input_assembly_sci.topology = topology;
        return *this;
    }

    /// Set the push constant ranges
    /// @param push_constant_ranges The push constant ranges
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_push_constant_ranges(std::vector<VkPushConstantRange> push_constant_ranges) {
        if (!push_constant_ranges.empty()) {
            throw InexorException("Error: Parameter 'push_constant_ranges' is empty!");
        }
        m_data.push_constant_ranges = std::move(push_constant_ranges);
        return *this;
    }

    /// Set the rasterization state of the graphics pipeline manually
    /// @param rasterization The rasterization state
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
        m_data.rasterization_sci = rasterization;
        return *this;
    }

    /// Set the scissor data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissors The scissors in in VkPipelineViewportStateCreateInfo
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_scissor(const VkRect2D &scissor) {
        m_data.scissors = {scissor};
        m_data.viewport_sci.scissorCount = 1;
        m_data.viewport_sci.pScissors = m_data.scissors.data();
        return *this;
    }

    /// Set the scissor data in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkRect2D)
    /// @param extent The extent of the scissor
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_scissor(const VkExtent2D &extent) {
        return set_scissor({
            // Convert VkExtent2D to VkRect2D
            .extent = extent,
        });
    }

    /// Set the shader modules
    /// @param shaders The shader stage create infos
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_shaders(std::vector<VkPipelineShaderStageCreateInfo> shaders) {
        if (shaders.empty()) {
            throw InexorException("Error: Parameter 'shaders' is empty!");
        }
        m_data.shader_stages = std::move(shaders);
        return *this;
    }

    /// Set the tesselation control point count
    /// @note This is not used in the code so far, because we are not using tesselation
    /// @param control_point_count The tesselation control point count
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_tesselation_control_point_count(const std::uint32_t control_point_count) {
        m_data.tesselation_sci.patchControlPoints = control_point_count;
        return *this;
    }

    /// Set the vertex input attribute descriptions manually
    /// @note As of C++23, there is no mechanism to do so called reflection in C++, meaning we can't get any information
    /// about the members of a struct at runtime, which would allow us to determine vertex input attributes
    /// automatically. Reflection was introduced in C++26, and once we switch to a newer C++ standard, we can use it.
    /// @param descriptions The vertex input attribute descriptions
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &
    set_vertex_input_attributes(const std::vector<VkVertexInputAttributeDescription> &descriptions) {
        if (descriptions.empty()) {
            throw InexorException("Error: Parameter 'descriptions' is empty!");
        }
        m_data.vertex_input_attribute_descriptions = descriptions;
        return *this;
    }

    /// Set the vertex input binding descriptions manually
    /// @param descriptions The vertex input binding descriptions
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &descriptions) {
        if (descriptions.empty()) {
            throw InexorException("Error: Parameter 'descriptions' is empty!");
        }
        m_data.vertex_input_binding_descriptions = descriptions;
        return *this;
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// There is another method called set_viewports in case multiple viewports will be used
    /// @param viewport The viewport in VkPipelineViewportStateCreateInfo
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_viewport(const VkViewport &viewport) {
        m_data.viewports = {viewport};
        m_data.viewport_sci.viewportCount = 1;
        m_data.viewport_sci.pViewports = m_data.viewports.data();
        return *this;
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkViewport)
    /// @param extent The extent of the viewport
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_viewport(const VkExtent2D &extent) {
        return set_viewport({
            // Convert VkExtent2D to VkViewport
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .maxDepth = 1.0f,
        });
    }

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A const reference to the ``this`` pointer, which allows method calls to be chained
    [[nodiscard]] auto &set_wireframe(const VkBool32 wireframe) {
        m_data.rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
