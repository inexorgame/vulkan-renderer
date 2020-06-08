#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

GPUMemoryBuffer::GPUMemoryBuffer(GPUMemoryBuffer &&other) noexcept
    : name(std::move(name)), device(std::exchange(other.device, nullptr)), buffer(std::exchange(other.buffer, nullptr)),
      allocation(std::exchange(other.allocation, nullptr)), allocation_info(std::move(other.allocation_info)),
      allocation_ci(std::move(other.allocation_ci)) {}

GPUMemoryBuffer::GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, const std::string &name,
                                 const VkDeviceSize &size, const VkBufferUsageFlags &buffer_usage,
                                 const VmaMemoryUsage &memory_usage)
    : device(device), vma_allocator(vma_allocator), name(name), buffer_size(size) {
    assert(device);
    assert(vma_allocator);
    assert(!name.empty());

    spdlog::debug("Creating GPU memory buffer of size {} for '{}'.", size, name);

    VkBufferCreateInfo buffer_ci = {};
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.size = size;
    buffer_ci.usage = buffer_usage;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocation_ci.usage = memory_usage;

#if VMA_RECORDING_ENABLED
    allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocation_ci.pUserData = this->name.data();
#else
    allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    // TODO: Should we create this buffer as mapped?
    // TODO: Is it good to have memory mapped all the time?
    // TODO: When should memory be mapped / unmapped?

    if (vmaCreateBuffer(vma_allocator, &buffer_ci, &allocation_ci, &buffer, &allocation, &allocation_info)) {
        throw std::runtime_error("Error: GPU memory buffer allocation for " + name + " failed!");
    }

    // Try to find the Vulkan debug marker function.
    auto *vkDebugMarkerSetObjectNameEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
        vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));

    if (vkDebugMarkerSetObjectNameEXT != nullptr) {
        // Since the function vkDebugMarkerSetObjectNameEXT has been found, we can assign an internal name for
        // debugging.
        VkDebugMarkerObjectNameInfoEXT name_info = {};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
        name_info.object = reinterpret_cast<std::uint64_t>(buffer);
        name_info.pObjectName = name.c_str();

        spdlog::debug("Assigning internal name '{}' to GPU memory buffer.", name);
        if (vkDebugMarkerSetObjectNameEXT(device, &name_info) != VK_SUCCESS) {
            throw std::runtime_error("Error: vkDebugMarkerSetObjectNameEXT failed for GPU memory buffer " + name + "!");
        }
    }
}

GPUMemoryBuffer::GPUMemoryBuffer(const VkDevice &device, const VmaAllocator &vma_allocator, const std::string &name,
                                 const VkDeviceSize &buffer_size, void *data, const std::size_t data_size,
                                 const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage)
    : GPUMemoryBuffer(device, vma_allocator, name, buffer_size, buffer_usage, memory_usage) {
    assert(device);
    assert(vma_allocator);
    assert(!name.empty());
    assert(buffer_size > 0);
    assert(data_size > 0);
    assert(data);

    // Copy the memory into the buffer!
    std::memcpy(allocation_info.pMappedData, data, data_size);
}

GPUMemoryBuffer::~GPUMemoryBuffer() {
    spdlog::trace("Destroying GPU memory buffer.");
    vmaDestroyBuffer(vma_allocator, buffer, allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
