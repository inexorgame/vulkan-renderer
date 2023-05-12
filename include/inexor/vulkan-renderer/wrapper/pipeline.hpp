#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

// TODO: Compute pipelines

/// RAII wrapper for VkPipeline
class GraphicsPipeline {
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

    [[nodiscard]] VkPipeline pipeline() const noexcept {
        return m_pipeline;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
