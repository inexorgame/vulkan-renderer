#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Semaphore::Semaphore(const Device &device, const std::string &name) : m_device(device), m_name(name) {
    assert(!name.empty());

    const auto semaphore_ci = make_info<VkSemaphoreCreateInfo>();

    if (const auto result = vkCreateSemaphore(m_device.device(), &semaphore_ci, nullptr, &m_semaphore);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSemaphore failed for " + m_name + " !", result);
    }
}

Semaphore::Semaphore(Semaphore &&other) noexcept : m_device(other.m_device) {
    m_semaphore = std::exchange(other.m_semaphore, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Semaphore::~Semaphore() {
    vkDestroySemaphore(m_device.device(), m_semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
