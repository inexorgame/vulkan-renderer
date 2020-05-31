#include "inexor/vulkan-renderer/wrapper/vma.hpp"

#define VMA_IMPLEMENTATION

// It makes memory of all new allocations initialized to bit pattern 0xDCDCDCDC.
// Before an allocation is destroyed, its memory is filled with bit pattern 0xEFEFEFEF.
// Memory is automatically mapped and unmapped if necessary.
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1

// Enforce specified number of bytes as a margin before and after every allocation.
#define VMA_DEBUG_MARGIN 16

// Enable validation of contents of the margins.
#define VMA_DEBUG_DETECT_CORRUPTION 1

#include <spdlog/spdlog.h>
#include <vma/vk_mem_alloc.h>

#include <fstream>

namespace inexor::vulkan_renderer::wrapper {

VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocator &&other) noexcept
    : vma_allocator(std::exchange(other.vma_allocator, nullptr)) {}

VulkanMemoryAllocator::VulkanMemoryAllocator(const VkInstance instance, const VkDevice device,
                                             const VkPhysicalDevice graphics_card) {
    assert(device);

    spdlog::debug("Initialising Vulkan memory allocator.");

    // Make sure to set the root directory of this repository as working directory in the debugger!
    // Otherwise, VMA won't be able to open this allocation replay file for writing.
    const std::string vma_replay_file = "vma-replays/vma_replay.csv";

    spdlog::debug("Opening VMA memory recording file for writing.");

    std::ofstream replay_file_test(vma_replay_file, std::ios::out);

    // Check if we can open the csv file.
    // This causes problems when the debugging path is set incorrectly!
    if (!replay_file_test) {
        throw std::runtime_error("Could not open VMA replay file " + vma_replay_file + " !");
    }

    spdlog::debug("VMA memory recording file opened successfully.");

    replay_file_test.close();

    // VMA allows to record memory allocations to a .csv file.
    // This .csv file can be replayed using tools from the repository.
    // This is very useful every time there is a bug in memory management.
    VmaRecordSettings vma_record_settings;

    // We flush the stream after every write operation because we are expecting unforseen program crashes.
    // This might has a negative effect on the application's performance but it's worth it for now.
    vma_record_settings.flags = VMA_RECORD_FLUSH_AFTER_CALL_BIT;
    vma_record_settings.pFilePath = vma_replay_file.c_str();

    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = graphics_card;
    allocator_info.instance = instance;
    allocator_info.device = device;
#if VMA_RECORDING_ENABLED
    allocator_info.pRecordSettings = &vma_record_settings;
#endif

    spdlog::debug("Creating Vulkan memory allocator instance.");

    if (vmaCreateAllocator(&allocator_info, &vma_allocator) != VK_SUCCESS) {
        throw std::runtime_error("Error: vmaCreateAllocator failed!");
    }

    spdlog::debug("Created VMA instance successfully!");
}

VulkanMemoryAllocator::~VulkanMemoryAllocator() {
    vmaDestroyAllocator(vma_allocator);
    vma_allocator = VK_NULL_HANDLE;
}

} // namespace inexor::vulkan_renderer::wrapper
