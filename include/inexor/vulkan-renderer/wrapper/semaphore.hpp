#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkSemaphore.
class Semaphore {
    const Device &m_device;
    VkSemaphore m_semaphore;
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the VkSemaphore.
    Semaphore(const Device &device, const std::string &name);
    Semaphore(const Semaphore &) = delete;
    Semaphore(Semaphore &&) noexcept;
    ~Semaphore();

    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

    [[nodiscard]] VkSemaphore get() const {
        return m_semaphore;
    }

    [[nodiscard]] const VkSemaphore *ptr() const {
        return &m_semaphore;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
