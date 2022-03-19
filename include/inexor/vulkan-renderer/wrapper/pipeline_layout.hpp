#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class PipelineLayout {
private:
    const Device &m_device;
    std::string m_name;

    VkPipelineLayout m_pipeline_layout;

public:
    PipelineLayout(const Device &device, const VkPipelineLayoutCreateInfo &pipeline_layout_ci, std::string name);

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&) noexcept = default;

    [[nodiscard]] VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper