#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Semaphore {
private:
    VkSemaphore m_semaphore;
    wrapper::Device &m_device;
    std::string m_name;

public:
    Semaphore(wrapper::Device &device, const std::string &name);
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
