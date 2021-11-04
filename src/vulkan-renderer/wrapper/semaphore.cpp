#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Semaphore::Semaphore(const Device &device, const std::string &name) : m_device(device), m_name(name) {
    assert(device.device());
    assert(!name.empty());

    auto semaphore_ci = make_info<VkSemaphoreCreateInfo>();
    if (const auto result = vkCreateSemaphore(device.device(), &semaphore_ci, nullptr, &m_semaphore);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSemaphore failed for " + name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}

Semaphore::Semaphore(Semaphore &&other) noexcept : m_device(other.m_device) {
    m_semaphore = std::exchange(other.m_semaphore, nullptr);
    m_name = std::move(other.m_name);
}

Semaphore::~Semaphore() {
    vkDestroySemaphore(m_device.device(), m_semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
