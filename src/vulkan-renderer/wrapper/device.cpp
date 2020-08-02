#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"

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
#include <vma/vma_usage.h>

#include <fstream>
#include <stdexcept>

namespace {

// TODO: Make proper use of queue priorities in the future.
constexpr float default_queue_priority = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

Device::Device(Device &&other) noexcept
    : device(std::exchange(other.device, nullptr)), graphics_card(std::exchange(other.graphics_card, nullptr)) {}

Device::Device(const VkInstance instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
               bool prefer_distinct_transfer_queue, const std::optional<std::uint32_t> preferred_physical_device_index)
    : graphics_card(graphics_card), surface(surface) {

    VulkanSettingsDecisionMaker settings_decision_maker;

    std::optional<VkPhysicalDevice> selected_graphics_card =
        settings_decision_maker.decide_which_graphics_card_to_use(instance, surface, preferred_physical_device_index);

    if (!selected_graphics_card) {
        throw std::runtime_error("Error: Could not find suitable graphics card!");
    }

    graphics_card = *selected_graphics_card;

    spdlog::debug("Creating Vulkan device queues.");

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    if (prefer_distinct_transfer_queue) {
        spdlog::debug("The application will try to use a distinct data transfer queue if it is available.");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        settings_decision_maker.find_queue_family_for_both_graphics_and_presentation(graphics_card, surface);

    if (queue_family_index_for_both_graphics_and_presentation) {
        spdlog::debug("One queue for both graphics and presentation will be used.");

        graphics_queue_family_index = *queue_family_index_for_both_graphics_and_presentation;
        present_queue_family_index = graphics_queue_family_index;

        // In this case, there is one queue family which can be used for both graphics and presentation.
        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = *queue_family_index_for_both_graphics_and_presentation;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);
    } else {
        spdlog::debug("No queue found which supports both graphics and presentation.");
        spdlog::debug("The application will try to use 2 separate queues.");

        // We have to use 2 different queue families.
        // One for graphics and another one for presentation.

        // Check which queue family index can be used for graphics.
        std::optional<std::uint32_t> queue_candidate =
            settings_decision_maker.find_graphics_queue_family(graphics_card);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for graphics!");
        }

        graphics_queue_family_index = *queue_candidate;

        // Check which queue family index can be used for presentation.
        queue_candidate = settings_decision_maker.find_presentation_queue_family(graphics_card, surface);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for presentation!");
        }

        present_queue_family_index = *queue_candidate;

        spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index);
        spdlog::debug("Presentation queue family index: {}.", present_queue_family_index);

        // Set up one queue for graphics.
        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = graphics_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);

        // Set up one queue for presentation.
        device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = present_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);
    }

    // Add another device queue just for data transfer.
    std::optional<std::uint32_t> queue_candidate =
        settings_decision_maker.find_distinct_data_transfer_queue_family(graphics_card);

    bool use_distinct_data_transfer_queue = false;

    if (queue_candidate && prefer_distinct_transfer_queue) {
        transfer_queue_family_index = *queue_candidate;

        spdlog::debug("A separate queue will be used for data transfer.");
        spdlog::debug("Data transfer queue family index: {}.", transfer_queue_family_index);

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = transfer_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);
    } else {
        // We don't have the opportunity to use a separated queue for data transfer!
        // Do not create a new queue, use the graphics queue instead.
        use_distinct_data_transfer_queue = false;
    }

    if (!use_distinct_data_transfer_queue) {
        spdlog::warn("The application is forces to avoid distinct data transfer queues.");
        spdlog::warn("Because of this, the graphics queue will be used for data transfer.");

        transfer_queue_family_index = graphics_queue_family_index;
    }

    std::vector<const char *> device_extensions_wishlist = {
        // Since we want to draw on a window, we need the swapchain extension.
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#ifndef NDEBUG
    if (enable_vulkan_debug_markers) {
        // Debug markers allow the assignment of internal names to Vulkan resources.
        // These internal names will conveniently be visible in debuggers like RenderDoc.
        // Debug markers are only available if RenderDoc is enabled.
        device_extensions_wishlist.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
#endif

    AvailabilityChecksManager availability_checks;

    std::vector<const char *> enabled_device_extensions;

    for (const auto &device_extension_name : device_extensions_wishlist) {
        if (availability_checks.has_device_extension(graphics_card, device_extension_name)) {
            spdlog::debug("Device extension '{}' is available on this system.", device_extension_name);
            enabled_device_extensions.push_back(device_extension_name);
        } else {
            throw std::runtime_error("Device extension " + std::string(device_extension_name) +
                                     " is not available on this system!");
        }
    }

    VkPhysicalDeviceFeatures used_features{};

    // Enable anisotropic filtering.
    used_features.samplerAnisotropy = VK_TRUE;

    auto device_ci = make_info<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size());
    device_ci.pQueueCreateInfos = queues_to_create.data();
    // Device layers were deprecated in Vulkan some time ago, essentially making all layers instance layers.
    device_ci.enabledLayerCount = 0;
    device_ci.ppEnabledLayerNames = nullptr;
    device_ci.enabledExtensionCount = static_cast<std::uint32_t>(enabled_device_extensions.size());
    device_ci.ppEnabledExtensionNames = enabled_device_extensions.data();
    device_ci.pEnabledFeatures = &used_features;

    spdlog::debug("Creating physical device (graphics card interface).");

    if (vkCreateDevice(graphics_card, &device_ci, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDevice failed!");
    }

#ifndef NDEBUG
    if (enable_vulkan_debug_markers) {
        spdlog::debug("Initializing Vulkan debug markers.");

        // The debug marker extension is not part of the core, so function pointers need to be loaded manually.
        vk_debug_marker_set_object_tag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(
            vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
        assert(vk_debug_marker_set_object_tag);

        vk_debug_marker_set_object_name = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
            vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
        assert(vk_debug_marker_set_object_name);

        vk_cmd_debug_marker_begin =
            reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
        assert(vk_cmd_debug_marker_begin);

        vk_cmd_debug_marker_end =
            reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
        assert(vk_cmd_debug_marker_end);

        vk_cmd_debug_marker_insert =
            reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));
        assert(vk_cmd_debug_marker_insert);

        vk_set_debug_utils_object_name = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
        assert(vk_set_debug_utils_object_name);
    } else {
        spdlog::warn("Warning: {} not present, debug markers are disabled.", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        spdlog::warn("Try running from inside a Vulkan graphics debugger (e.g. RenderDoc).");
    }
#endif

    spdlog::debug("Initialising GPU queues.");
    spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index);
    spdlog::debug("Presentation queue family index: {}.", present_queue_family_index);
    spdlog::debug("Data transfer queue family index: {}.", transfer_queue_family_index);

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
    vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(device, transfer_queue_family_index, 0, &transfer_queue);
    }

    spdlog::debug("Creating vma allocator");

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

    VmaAllocatorCreateInfo vma_allocator_ci{};
    vma_allocator_ci.physicalDevice = graphics_card;
    vma_allocator_ci.instance = instance;
    vma_allocator_ci.device = device;
#if VMA_RECORDING_ENABLED
    vma_allocator_ci.pRecordSettings = &vma_record_settings;
#endif

    spdlog::debug("Creating Vulkan memory allocator instance.");

    if (vmaCreateAllocator(&vma_allocator_ci, &m_allocator) != VK_SUCCESS) {
        throw std::runtime_error("Error: vmaCreateAllocator failed!");
    }

    spdlog::debug("Created device successfully.");
}

Device::~Device() {
    assert(device);
    spdlog::trace("Destroying device.");
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(device, nullptr);
}

#ifndef NDEBUG
void Device::set_object_name(const std::uint64_t object, const VkDebugReportObjectTypeEXT type,
                             const std::string &name) {
    assert(device);
    assert(!name.empty());
    assert(object);
    assert(vk_debug_marker_set_object_name);

    auto name_info = make_info<VkDebugMarkerObjectNameInfoEXT>();
    name_info.objectType = type;
    name_info.object = object;
    name_info.pObjectName = name.c_str();

    vk_debug_marker_set_object_name(device, &name_info);
}

void Device::set_object_tag(const std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::uint64_t name,
                            const std::size_t tag_size, const void *tag) {
    assert(device);
    assert(name);
    assert(tag_size > 0);
    assert(tag);
    assert(vk_debug_marker_set_object_tag);

    auto tagInfo = make_info<VkDebugMarkerObjectTagInfoEXT>();
    tagInfo.objectType = type;
    tagInfo.object = object;
    tagInfo.tagName = name;
    tagInfo.tagSize = tag_size;
    tagInfo.pTag = tag;

    vk_debug_marker_set_object_tag(device, &tagInfo);
}

void Device::bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                               const std::array<float, 4> color) {
    assert(command_buffer);
    assert(!name.empty());
    assert(vk_cmd_debug_marker_begin);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();
    debug_marker.color[0] = color[0];
    debug_marker.color[1] = color[1];
    debug_marker.color[2] = color[2];
    debug_marker.color[3] = color[3];
    debug_marker.pMarkerName = name.c_str();

    vk_cmd_debug_marker_begin(command_buffer, &debug_marker);
}

void Device::insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                                 const std::array<float, 4> color) {
    assert(command_buffer);
    assert(!name.empty());
    assert(vk_cmd_debug_marker_insert);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();
    debug_marker.color[0] = color[0];
    debug_marker.color[1] = color[1];
    debug_marker.color[2] = color[2];
    debug_marker.color[3] = color[3];
    debug_marker.pMarkerName = name.c_str();

    vk_cmd_debug_marker_insert(command_buffer, &debug_marker);
}

void Device::end_debug_region(const VkCommandBuffer command_buffer) {
    assert(vk_cmd_debug_marker_end);
    vk_cmd_debug_marker_end(command_buffer);
}
#endif

} // namespace inexor::vulkan_renderer::wrapper
