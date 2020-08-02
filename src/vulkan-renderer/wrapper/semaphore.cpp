#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Semaphore::Semaphore(Semaphore &&other) noexcept
    : m_device(other.m_device), m_semaphore(std::exchange(other.m_semaphore, nullptr)),
      m_name(std::move(other.m_name)) {}

Semaphore::Semaphore(const VkDevice device, const std::string &name) : m_device(device), m_name(name) {
    assert(device);
    assert(!name.empty());

    spdlog::debug("Creating semaphore {}.", name);

    auto semaphore_ci = make_info<VkSemaphoreCreateInfo>();
    if (vkCreateSemaphore(device, &semaphore_ci, nullptr, &m_semaphore) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSemaphore failed for " + name + " !");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    spdlog::debug("Created semaphore successfully.");
}

Semaphore::~Semaphore() {
    spdlog::trace("Destroying semaphore {}.", m_name);
    vkDestroySemaphore(m_device, m_semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
