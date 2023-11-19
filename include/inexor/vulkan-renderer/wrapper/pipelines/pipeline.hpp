#pragma once

#include <volk.h>

#include <string>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Compute pipelines

/// RAII wrapper for VkPipeline
class GraphicsPipeline {
    friend class CommandBuffer;

private:
    const Device &m_device;
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_ci The pipeline create info
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device, const VkGraphicsPipelineCreateInfo &pipeline_ci, std::string name);
    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
