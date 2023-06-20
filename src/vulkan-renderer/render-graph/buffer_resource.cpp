#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"

#include "inexor/vulkan-renderer/render-graph/descriptor_set_update_frequency_category.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

BufferResource::BufferResource(std::string name, BufferUsage usage,
                               DescriptorSetUpdateFrequencyCategory update_frequency, std::function<void()> on_update)
    : m_name(std::move(name)), m_usage(usage), m_update_frequency(m_update_frequency),
      m_on_update(std::move(on_update)) {
    if (usage != BufferUsage::UNIFORM_BUFFER) {
        m_requires_staging_buffer_update = true;
    }
}

BufferResource::BufferResource(BufferResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_update_frequency = other.m_update_frequency;
    m_buffer = std::exchange(other.m_buffer, nullptr);
    m_on_update = std::move(other.m_on_update);
    m_update_required = other.m_update_required;
    m_data = std::exchange(other.m_data, nullptr);
    m_data_size = other.m_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
