#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class GraphicsPipeline
/// @brief RAII wrapper class for VkPipeline.
class GraphicsPipeline {
private:
    const Device &m_device;
    VkPipeline graphics_pipeline;
    VkPipelineCache pipeline_cache;
    std::string name;

public:
    /// @brief Constructs the graphics pipeline.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param pipeline_layout [in] The layout of the graphics pipeline.
    /// @param render_pass [in] The associated renderpass.
    /// @param shader_stages [in] The shader stages which will be used.
    /// @param vertex_binding [in] The vertex input binding descriptions.
    /// @param attribute_binding [in] The vertex input attribute descriptions.
    /// @param window_width [in] The width of the window.
    /// @param window_height [in] The height of the window.
    /// @param name [in] The internal debug marker name of the graphics pipeline.
    GraphicsPipeline(const Device &device, const VkPipelineLayout pipeline_layout, const VkRenderPass render_pass,
                     const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                     const std::vector<VkVertexInputBindingDescription> &vertex_binding,
                     const std::vector<VkVertexInputAttributeDescription> &attribute_binding,
                     const std::uint32_t window_width, const std::uint32_t window_height, const std::string &name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&other) noexcept;

    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) noexcept = default;

    [[nodiscard]] VkPipeline get() const {
        return graphics_pipeline;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
