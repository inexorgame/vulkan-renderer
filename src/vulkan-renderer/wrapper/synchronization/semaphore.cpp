#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::synchronization {

Semaphore::Semaphore(const Device &device, const std::string &name) : m_device(device), m_name(name) {
    assert(!name.empty());

    const auto semaphore_ci = make_info<VkSemaphoreCreateInfo>();

    if (const auto result = vkCreateSemaphore(m_device.device(), &semaphore_ci, nullptr, &m_semaphore);
        result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkCreateSemaphore failed!", result, m_name);
    }
    m_device.set_debug_name(m_semaphore, m_name);
}

Semaphore::Semaphore(Semaphore &&other) noexcept : m_device(other.m_device) {
    m_semaphore = std::exchange(other.m_semaphore, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Semaphore::~Semaphore() {
    vkDestroySemaphore(m_device.device(), m_semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::synchronization
