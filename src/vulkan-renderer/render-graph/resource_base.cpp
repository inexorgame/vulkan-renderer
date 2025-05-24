#include "inexor/vulkan-renderer/render-graph/resource_base.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::InexorException;

ResourceBase::ResourceBase(const Device &device, std::string name) : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
}

ResourceBase::~ResourceBase() {
    destroy_staging_buffer();
}

void ResourceBase::destroy_staging_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_staging_buffer_alloc);
    m_staging_buffer = VK_NULL_HANDLE;
    m_staging_buffer_alloc = VK_NULL_HANDLE;
}

void ResourceBase::request_update(void *src_data, const std::size_t src_data_size) {
    if (src_data == nullptr || src_data_size == 0) {
        return;
    }
    m_src_data = src_data;
    m_src_data_size = src_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
