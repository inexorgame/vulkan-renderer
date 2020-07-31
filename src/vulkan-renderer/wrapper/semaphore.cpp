#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Semaphore::Semaphore(Semaphore &&other) noexcept
    : device(other.device), semaphore(std::exchange(other.semaphore, nullptr)), name(std::move(other.name)) {}

Semaphore::Semaphore(const VkDevice device, const std::string &name) : device(device), name(name) {
    assert(device);
    assert(!name.empty());

    spdlog::debug("Creating semaphore {}.", name);

    auto semaphore_ci = make_info<VkSemaphoreCreateInfo>();
    if (vkCreateSemaphore(device, &semaphore_ci, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSemaphore failed for " + name + " !");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    spdlog::debug("Created semaphore successfully.");
}

Semaphore::~Semaphore() {
    spdlog::trace("Destroying semaphore {}.", name);
    vkDestroySemaphore(device, semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
