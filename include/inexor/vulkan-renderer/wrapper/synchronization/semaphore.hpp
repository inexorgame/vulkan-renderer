#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::synchronization {

/// RAII wrapper class for VkSemaphore
class Semaphore {
    const Device &m_device;
    VkSemaphore m_semaphore{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the VkSemaphore.
    Semaphore(const Device &device, const std::string &name);

    Semaphore(const Semaphore &) = delete;
    Semaphore(Semaphore &&) noexcept;

    ~Semaphore();

    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

    [[nodiscard]] const VkSemaphore *semaphore() const {
        return &m_semaphore;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
