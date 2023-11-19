#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_frequency.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

BufferResource::BufferResource(std::string name, const BufferType type,
                               const DescriptorSetUpdateFrequency update_frequency,
                               std::optional<std::function<void()>> on_update)
    : m_name(std::move(name)), m_type(type), m_update_frequency(update_frequency), m_on_update(std::move(on_update)) {
    if (type != BufferType::UNIFORM_BUFFER) {
        m_requires_staging_buffer_update = true;
    }
}

BufferResource::BufferResource(BufferResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_type = other.m_type;
    m_update_frequency = other.m_update_frequency;
    m_buffer = std::exchange(other.m_buffer, nullptr);
    m_on_update = std::move(other.m_on_update);
    m_update_required = other.m_update_required;
    m_data = std::exchange(other.m_data, nullptr);
    m_data_size = other.m_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
