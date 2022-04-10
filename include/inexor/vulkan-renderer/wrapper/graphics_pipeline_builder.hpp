#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// A builder pattern class for building GraphicsPipeline wrapper instances
class GraphicsPipelineBuilder {
private:
    const Device &m_device;

    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_sci{make_info<VkPipelineInputAssemblyStateCreateInfo>()};
    VkPipelineViewportStateCreateInfo m_viewport_sci{make_info<VkPipelineViewportStateCreateInfo>()};
    VkPipelineRasterizationStateCreateInfo m_rasterization_sci{make_info<VkPipelineRasterizationStateCreateInfo>()};
    VkPipelineMultisampleStateCreateInfo m_multisample_sci{make_info<VkPipelineMultisampleStateCreateInfo>()};
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_sci{make_info<VkPipelineDepthStencilStateCreateInfo>()};
    VkPipelineColorBlendStateCreateInfo m_color_blend_sci{make_info<VkPipelineColorBlendStateCreateInfo>()};
    VkPipelineDynamicStateCreateInfo m_dynamic_states_ci{make_info<VkPipelineDynamicStateCreateInfo>()};

    std::vector<VkVertexInputBindingDescription> m_vertex_input_bindings;
    std::vector<VkVertexInputAttributeDescription> m_vertex_input_attributes;

public:
    GraphicsPipelineBuilder(const Device &device);

    GraphicsPipelineBuilder(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder(GraphicsPipelineBuilder &&) noexcept;

    ~GraphicsPipelineBuilder() = default;

    GraphicsPipelineBuilder &operator=(const GraphicsPipelineBuilder &) = delete;
    GraphicsPipelineBuilder &operator=(GraphicsPipelineBuilder &&) noexcept = default;

    // TODO: Use C++20: std::span!
    [[nodiscard]] GraphicsPipelineBuilder &set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states);

    [[nodiscard]] GraphicsPipelineBuilder &set_scissor_count(std::uint32_t scissor_count);

    [[nodiscard]] GraphicsPipelineBuilder &set_viewport_count(std::uint32_t viewport_count);

    // TODO: Use C++20: std::span!
    [[nodiscard]] GraphicsPipelineBuilder &
    set_color_blend_attachments(const std::vector<VkPipelineColorBlendAttachmentState> &attachments);

    [[nodiscard]] GraphicsPipelineBuilder &
    set_vertex_input_attributes(const std::vector<VkVertexInputAttributeDescription> &vertex_input_attributes);

    [[nodiscard]] GraphicsPipelineBuilder &
    set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &vertex_input_bindings);

    [[nodiscard]] GraphicsPipeline build(const PipelineLayout &m_pipeline_layout, const RenderPass &m_renderpass,
                                         const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                                         std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper
