#pragma once

#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <memory>

namespace inexor::vulkan_renderer::render_components {

// Using declarations
using wrapper::Shader;
using wrapper::pipelines::GraphicsPipeline;

class MaterialBase {
protected:
    std::shared_ptr<Shader> m_vertex_shader;
    std::shared_ptr<Shader> m_fragment_shader;
    std::weak_ptr<GraphicsPipeline> m_graphics_pipeline;
    // TODO: How would the descriptor set layout be created in a renderer class?
    // In other words: should descriptor set layout creation be separate from renderer?
    // Should it be more close to any specific renderer, or to any specific material?
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
};

} // namespace inexor::vulkan_renderer::render_components
