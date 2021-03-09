#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkPipeline.
class GraphicsPipeline {
    const Device &m_device;
    VkPipeline graphics_pipeline{VK_NULL_HANDLE};
    VkPipelineCache pipeline_cache{VK_NULL_HANDLE};
    std::string name;

public:
    /// @brief Construct the graphics pipeline.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param pipeline_layout The layout of the graphics pipeline.
    /// @param render_pass The associated renderpass.
    /// @param shader_stages The shader stages which will be used.
    /// @param vertex_binding The vertex input binding descriptions.
    /// @param attribute_binding The vertex input attribute descriptions.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    /// @param name The internal debug marker name of the graphics pipeline.
    GraphicsPipeline(const Device &device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass,
                     const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                     const std::vector<VkVertexInputBindingDescription> &vertex_binding,
                     const std::vector<VkVertexInputAttributeDescription> &attribute_binding,
                     std::uint32_t window_width, std::uint32_t window_height, const std::string &name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&other) noexcept;

    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

    [[nodiscard]] VkPipeline get() const {
        return graphics_pipeline;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
