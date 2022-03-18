#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Shader;

// TODO: Use C++209 std::span for all make_infos which accept a std::vector!

/// @brief A small helper function that return vulkan create infos with sType already set
/// @code{.cpp}
/// auto render_pass_ci = make_info<VkRenderPassCreateInfo>();
/// @endcode
/// @note Also zeros the returned struct
template <typename T>
[[nodiscard]] T make_info();

///
///
///
///
///
///
///
///
///
///
///
///
[[nodiscard]] VkGraphicsPipelineCreateInfo make_info(const VkPipelineLayout pipeline_layout,
                                                     const VkRenderPass renderpass,
                                                     const std::vector<VkPipelineShaderStageCreateInfo> &stages,
                                                     const VkPipelineVertexInputStateCreateInfo *vertex_input_state,
                                                     const VkPipelineInputAssemblyStateCreateInfo *input_assembly_state,
                                                     const VkPipelineViewportStateCreateInfo *viewport_state,
                                                     const VkPipelineRasterizationStateCreateInfo *rasterization_state,
                                                     const VkPipelineMultisampleStateCreateInfo *multisample_state,
                                                     const VkPipelineDepthStencilStateCreateInfo *depth_stencil_state,
                                                     const VkPipelineColorBlendStateCreateInfo *color_blend_state,
                                                     const VkPipelineDynamicStateCreateInfo *dynamic_state);

///
///
[[nodiscard]] VkPipelineDynamicStateCreateInfo make_info(const std::vector<VkDynamicState> &dynamic_states);

///
///
///
[[nodiscard]] VkPipelineVertexInputStateCreateInfo
make_info(const std::vector<VkVertexInputBindingDescription> &vertex_input_binding_descriptions,
          const std::vector<VkVertexInputAttributeDescription> &vertex_input_attribute_descriptions);

///
///
///
[[nodiscard]] VkPipelineLayoutCreateInfo make_info(const std::vector<VkDescriptorSetLayout> &set_layouts,
                                                   const std::vector<VkPushConstantRange> &push_constant_ranges = {});

///
///
///
[[nodiscard]] VkPipelineShaderStageCreateInfo make_info(const wrapper::Shader &shader);

///
///
[[nodiscard]] VkImageCreateInfo make_info(VkFormat format);

///
///
///
///
///
[[nodiscard]] VkRenderPassCreateInfo make_info(const std::vector<VkAttachmentDescription> &attachments,
                                               const std::vector<VkSubpassDescription> &subpasses,
                                               const std::vector<VkSubpassDependency> &dependencies);

} // namespace inexor::vulkan_renderer::wrapper
