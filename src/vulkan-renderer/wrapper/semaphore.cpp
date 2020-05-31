#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Semaphore::Semaphore(Semaphore &&other) noexcept
    : device(other.device), semaphore(std::exchange(other.semaphore, nullptr)), name(std::move(other.name)) {}

Semaphore::Semaphore(const VkDevice device, const std::string &name) : device(device), name(name) {
    assert(device);
    assert(!name.empty());

    // So far, there is nothing to fill into this structure.
    // This may change in the future!
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.flags = 0;

    spdlog::debug("Creating semaphore {}.", name);

    if (vkCreateSemaphore(device, &create_info, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSemaphore failed for " + name + " !");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    spdlog::debug("Created semaphore successfully.");
}

Semaphore::~Semaphore() {
    vkDestroySemaphore(device, semaphore, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
