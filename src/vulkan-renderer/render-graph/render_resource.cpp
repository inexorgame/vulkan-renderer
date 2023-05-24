#include "inexor/vulkan-renderer/render-graph/render_resource.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

RenderResource::RenderResource(const wrapper::Device &device, std::string name)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        std::invalid_argument("Error: Name of render resource is empty!");
    }
}

} // namespace inexor::vulkan_renderer::render_graph
