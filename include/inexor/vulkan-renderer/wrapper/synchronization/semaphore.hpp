#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::synchronization {

// Using declaration
using render_graph::RenderGraph;
using wrapper::Swapchain;

/// RAII wrapper class for VkSemaphore
class Semaphore {
    friend class RenderGraph;
    friend class Swapchain;

private:
    const Device &m_device;
    VkSemaphore m_semaphore{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The const reference to a device RAII wrapper instance
    /// @param name The internal debug marker name of the VkSemaphore
    Semaphore(const Device &device, const std::string &name);
    Semaphore(const Semaphore &) = delete;
    Semaphore(Semaphore &&) noexcept;
    ~Semaphore();

    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
