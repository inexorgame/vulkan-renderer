#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Semaphore {
private:
    VkSemaphore m_semaphore;
    VkDevice m_device;
    std::string m_name;

public:
    /// Delete the copy constructor so Vulkan semaphores are move-only objects.
    Semaphore(const Semaphore &) = delete;
    Semaphore(Semaphore &&other) noexcept;

    /// Delete the move assignment operator so Vulkan semaphores are move-only objects.
    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore &operator=(Semaphore &&) noexcept = default;

    Semaphore(const VkDevice device, const std::string &name);

    ~Semaphore();

    [[nodiscard]] VkSemaphore get() const {
        return m_semaphore;
    }

    [[nodiscard]] const VkSemaphore *ptr() const {
        return &m_semaphore;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
