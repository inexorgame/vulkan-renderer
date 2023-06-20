#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

namespace inexor::vulkan_renderer::wrapper::pipelines {

/// RAII wrapper class for VkPipelineLayout
class PipelineLayout {
private:
    const Device &m_device;
    std::string m_name;
    VkPipelineLayout m_pipeline_layout;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_layout_ci The pipeline layout create info
    /// @param name The name of the pipeline layout
    PipelineLayout(const Device &device, const VkPipelineLayoutCreateInfo &pipeline_layout_ci, std::string name);

    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layouts The descriptor set layouts
    /// @param push_constant_ranges The push constant ranges
    /// @param name The name of the pipeline layout
    PipelineLayout(const Device &device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                   const std::vector<VkPushConstantRange> &push_constant_ranges, std::string name);

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept = delete;

    [[nodiscard]] VkPipelineLayout pipeline_layout() const noexcept {
        return m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
