#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Semaphore {
private:
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
        assert(m_semaphore);
        return m_semaphore;
    }

    [[nodiscard]] const VkSemaphore *ptr() const {
        assert(m_semaphore);
        return &m_semaphore;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
