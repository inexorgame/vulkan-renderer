#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const Device &device, const PipelineCache &pipeline_cache)
    : m_device(device), m_descriptor_set_allocator(device), m_write_descriptor_set_builder(device),
      m_graphics_pipeline_builder(device, pipeline_cache) {}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type,
                                              std::function<void()> on_update) {
    return m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update)));
}

std::weak_ptr<GraphicsPass> RenderGraph::add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass) {
    return m_graphics_passes.emplace_back(std::move(graphics_pass));
}

void RenderGraph::add_graphics_pipeline(OnCreateGraphicsPipeline on_create_graphics_pipeline) {
    m_graphics_pipeline_create_functions.emplace_back(std::move(on_create_graphics_pipeline));
}

void RenderGraph::add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                          OnAllocateDescriptorSet on_allocate_descriptor_set,
                                          OnBuildWriteDescriptorSets on_update_descriptor_set) {
    m_resource_descriptors.emplace_back(std::move(on_build_descriptor_set_layout),
                                        std::move(on_allocate_descriptor_set), std::move(on_update_descriptor_set));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureUsage usage, const VkFormat format,
                                                const std::uint32_t width, const std::uint32_t height,
                                                const std::uint32_t channels, const VkSampleCountFlagBits sample_count,
                                                std::function<void()> on_update) {
    return m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(name), usage, format, width, height,
                                                             channels, sample_count, std::move(on_update)));
}

void RenderGraph::compile() {
    //
}

void RenderGraph::reset() {
    m_buffers.clear();
    m_textures.clear();
    m_graphics_passes.clear();
    m_resource_descriptors.clear();
}

} // namespace inexor::vulkan_renderer::render_graph
