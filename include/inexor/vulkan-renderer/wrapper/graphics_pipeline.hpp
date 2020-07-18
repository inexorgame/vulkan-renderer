#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class GraphicsPipeline {
private:
    VkDevice device;
    VkPipeline graphics_pipeline;
    VkPipelineCache pipeline_cache;
    std::string name;

public:
    /// Delete the copy constructor so graphics pipelines are move-only objects.
    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&other) noexcept;

    /// Delete the copy assignment operator so graphics pipelines are move-only objects.
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) noexcept = default;

    /// @brief Creates a graphics pipeline.
    /// @param device [in] The Vulkan device.
    /// @param pipeline_layout [in] The pipeline layout.
    /// @param render_pass [in] The associated renderpass.
    /// @param shader_stages [in] The shaders and the stages they are used in.
    /// @param vertex_binding [in] The vertex binding description.
    /// @param attribute_binding [in] The vertex attribute binding description.
    /// @param window_width [in] The width of the window.
    /// @param window_height [in] The height of the window.
    /// @param name [in] The internal name of the graphics pipeline.
    GraphicsPipeline(const VkDevice device, const VkPipelineLayout pipeline_layout, const VkRenderPass render_pass,
                     const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                     const VkVertexInputBindingDescription vertex_binding,
                     const std::vector<VkVertexInputAttributeDescription> &attribute_binding,
                     const std::uint32_t window_width, const std::uint32_t window_height, const std::string &name);

    ~GraphicsPipeline();

    [[nodiscard]] VkPipeline get() const {
        return graphics_pipeline;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
