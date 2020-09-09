#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class Semaphore {
    const Device &m_device;
    VkSemaphore m_semaphore;
    const std::string m_name;

public:
    Semaphore(const Device &device, const std::string &name);
    Semaphore(const Semaphore &) = delete;
    Semaphore(Semaphore &&) noexcept;
    ~Semaphore();

    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) = default;

    [[nodiscard]] VkSemaphore get() const {
        return m_semaphore;
    }

    [[nodiscard]] const VkSemaphore *ptr() const {
        return &m_semaphore;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
