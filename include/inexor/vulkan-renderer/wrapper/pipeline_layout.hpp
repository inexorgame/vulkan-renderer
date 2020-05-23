#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class PipelineLayout {
private:
    VkDevice device;
    VkPipelineLayout pipeline_layout;
    std::string name;

public:
    /// Delete the copy constructor so pipeline layouts are move-only objects.
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&other) noexcept;

    /// Delete the move-assignment operator so pipeline-layouts are move-only objects.
    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept = default;

    /// @brief Creates a pipeline layout
    /// @param device [in] The Vulkan device.
    /// @param descriptor_set_layouts [in] The descriptor set layouts for the pipeline layout.
    /// @param name [in] The internal name of the pipeline layout.
    PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts, const std::string &name);

    ~PipelineLayout();

    [[nodiscard]] VkPipelineLayout get() const {
        return pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
