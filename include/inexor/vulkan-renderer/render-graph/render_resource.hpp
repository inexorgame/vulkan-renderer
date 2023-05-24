#pragma once

#include "inexor/vulkan-renderer/render-graph/render_graph_object.hpp"

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

/// A base class for resources in the rendergraph
class RenderResource : public RenderGraphObject {
private:
    const wrapper::Device &m_device;
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the render resource (must not be empty!)
    /// @exception std::invalid_argument Internal denug name is empty
    RenderResource(const wrapper::Device &device, std::string name);
    RenderResource(const RenderResource &) = delete;
    RenderResource(RenderResource &&) noexcept;
    ~RenderResource() = default;

    RenderResource &operator=(const RenderResource &) = delete;
    RenderResource &operator=(RenderResource &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
