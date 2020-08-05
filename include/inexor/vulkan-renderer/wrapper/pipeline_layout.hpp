#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class PipelineLayout {
private:
    VkDevice m_device;
    VkPipelineLayout m_pipeline_layout;
    std::string m_name;

public:
    /// @brief Creates a pipeline layout
    /// @param device [in] The Vulkan device.
    /// @param descriptor_set_layouts [in] The descriptor set layouts for the pipeline layout.
    /// @param name [in] The internal name of the pipeline layout.
    PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                   const std::string &name);
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&) = default;

    [[nodiscard]] VkPipelineLayout get() const {
        return m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
