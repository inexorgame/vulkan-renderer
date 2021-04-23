#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

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
#include <vk_mem_alloc.h>

#include <algorithm>
#include <fstream>

namespace {

// TODO: Make proper use of queue priorities in the future.
constexpr float default_queue_priority = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

bool Device::is_extension_supported(const VkPhysicalDevice graphics_card, const std::string &extension_name) {
    assert(graphics_card);
    assert(!extension_name.empty());

    std::uint32_t device_extension_count = 0;

    // Query how many device extensions are available.
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    if (device_extension_count == 0) {
        throw std::runtime_error("Error: No Vulkan device extensions available!");
    }

    std::vector<VkExtensionProperties> device_extensions(device_extension_count);

    // Store all available device extensions.
    if (const auto result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count,
                                                                 device_extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    // Search for the requested device extension.
    return std::find_if(device_extensions.begin(), device_extensions.end(),
                        [&](const VkExtensionProperties device_extension) {
                            return device_extension.extensionName == extension_name;
                        }) != device_extensions.end();
}

bool Device::is_layer_supported(const VkPhysicalDevice graphics_card, const std::string &layer_name) {
    assert(graphics_card);
    assert(!layer_name.empty());

    std::uint32_t device_layer_count = 0;

    // Query how many device layers are available.
    if (const auto result = vkEnumerateDeviceLayerProperties(graphics_card, &device_layer_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceLayerProperties failed!", result);
    }

    if (device_layer_count == 0) {
        throw std::runtime_error("Error: No Vulkan device extensions available!");
    }

    std::vector<VkLayerProperties> device_layers(device_layer_count);

    // Store all available device layers.
    if (const auto result = vkEnumerateDeviceLayerProperties(graphics_card, &device_layer_count, device_layers.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceLayerProperties failed!", result);
    }

    // Search for the requested instance extensions.
    return std::find_if(device_layers.begin(), device_layers.end(), [&](const VkLayerProperties device_extension) {
               return device_extension.layerName == layer_name;
           }) != device_layers.end();
}

bool Device::is_swapchain_supported(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);
    return is_extension_supported(graphics_card, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

bool Device::is_presentation_supported(const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    VkBool32 presentation_supported = false;

    // Query if presentation is supported.
    if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_supported);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
    }

    return presentation_supported;
}

Device::Device(const VkInstance instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
               bool prefer_distinct_transfer_queue, const std::optional<std::uint32_t> preferred_physical_device_index)
    : m_surface(surface), m_enable_vulkan_debug_markers(enable_vulkan_debug_markers) {

    VulkanSettingsDecisionMaker settings_decision_maker;

    std::optional<VkPhysicalDevice> selected_graphics_card =
        settings_decision_maker.decide_which_graphics_card_to_use(instance, surface, preferred_physical_device_index);

    if (!selected_graphics_card) {
        throw std::runtime_error("Error: Could not find suitable graphics card!");
    }

    m_graphics_card = *selected_graphics_card;

    VkPhysicalDeviceProperties graphics_card_properties;

    // Get the information about that graphics card's properties.
    vkGetPhysicalDeviceProperties(m_graphics_card, &graphics_card_properties);

    spdlog::debug("Creating device using graphics card: {}.", graphics_card_properties.deviceName);

    m_gpu_name = graphics_card_properties.deviceName;

    spdlog::debug("Creating Vulkan device queues.");

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    if (prefer_distinct_transfer_queue) {
        spdlog::debug("The application will try to use a distinct data transfer queue if it is available.");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        settings_decision_maker.find_queue_family_for_both_graphics_and_presentation(m_graphics_card, surface);

    if (queue_family_index_for_both_graphics_and_presentation) {
        spdlog::debug("One queue for both graphics and presentation will be used.");

        m_graphics_queue_family_index = *queue_family_index_for_both_graphics_and_presentation;
        m_present_queue_family_index = m_graphics_queue_family_index;

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
            settings_decision_maker.find_graphics_queue_family(m_graphics_card);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for graphics!");
        }

        m_graphics_queue_family_index = *queue_candidate;

        // Check which queue family index can be used for presentation.
        queue_candidate = settings_decision_maker.find_presentation_queue_family(m_graphics_card, surface);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for presentation!");
        }

        m_present_queue_family_index = *queue_candidate;

        spdlog::debug("Graphics queue family index: {}.", m_graphics_queue_family_index);
        spdlog::debug("Presentation queue family index: {}.", m_present_queue_family_index);

        // Set up one queue for graphics.
        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_graphics_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);

        // Set up one queue for presentation.
        device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_present_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::default_queue_priority;

        queues_to_create.push_back(device_queue_ci);
    }

    // Add another device queue just for data transfer.
    std::optional<std::uint32_t> queue_candidate =
        settings_decision_maker.find_distinct_data_transfer_queue_family(m_graphics_card);

    bool use_distinct_data_transfer_queue = false;

    if (queue_candidate && prefer_distinct_transfer_queue) {
        m_transfer_queue_family_index = *queue_candidate;

        spdlog::debug("A separate queue will be used for data transfer.");
        spdlog::debug("Data transfer queue family index: {}.", m_transfer_queue_family_index);

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_transfer_queue_family_index;
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

        m_transfer_queue_family_index = m_graphics_queue_family_index;
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

    std::vector<const char *> enabled_device_extensions;

    for (const auto &device_extension_name : device_extensions_wishlist) {
        if (is_extension_supported(m_graphics_card, device_extension_name)) {
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

    if (const auto result = vkCreateDevice(m_graphics_card, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

#ifndef NDEBUG
    if (m_enable_vulkan_debug_markers) {
        spdlog::debug("Initializing Vulkan debug markers.");

        // The debug marker extension is not part of the core, so function pointers need to be loaded manually.
        m_vk_debug_marker_set_object_tag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(
            vkGetDeviceProcAddr(m_device, "vkDebugMarkerSetObjectTagEXT"));
        assert(m_vk_debug_marker_set_object_tag);

        m_vk_debug_marker_set_object_name = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
            vkGetDeviceProcAddr(m_device, "vkDebugMarkerSetObjectNameEXT"));
        assert(m_vk_debug_marker_set_object_name);

        m_vk_cmd_debug_marker_begin =
            reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerBeginEXT"));
        assert(m_vk_cmd_debug_marker_begin);

        m_vk_cmd_debug_marker_end =
            reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerEndEXT"));
        assert(m_vk_cmd_debug_marker_end);

        m_vk_cmd_debug_marker_insert =
            reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerInsertEXT"));
        assert(m_vk_cmd_debug_marker_insert);

        m_vk_set_debug_utils_object_name = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetDeviceProcAddr(m_device, "vkSetDebugUtilsObjectNameEXT"));
        assert(m_vk_set_debug_utils_object_name);
    } else {
        spdlog::warn("Warning: {} not present, debug markers are disabled.", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        spdlog::warn("Try running from inside a Vulkan graphics debugger (e.g. RenderDoc).");
    }
#endif

    spdlog::debug("Initialising GPU queues.");
    spdlog::debug("Graphics queue family index: {}.", m_graphics_queue_family_index);
    spdlog::debug("Presentation queue family index: {}.", m_present_queue_family_index);
    spdlog::debug("Data transfer queue family index: {}.", m_transfer_queue_family_index);

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(m_device, m_present_queue_family_index, 0, &m_present_queue);
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(m_device, m_transfer_queue_family_index, 0, &m_transfer_queue);
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
    vma_allocator_ci.physicalDevice = m_graphics_card;
    vma_allocator_ci.instance = instance;
    vma_allocator_ci.device = m_device;
#if VMA_RECORDING_ENABLED
    vma_allocator_ci.pRecordSettings = &vma_record_settings;
#endif

    spdlog::debug("Creating Vulkan memory allocator instance.");

    if (const auto result = vmaCreateAllocator(&vma_allocator_ci, &m_allocator); result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }

    spdlog::debug("Created device successfully.");
}

Device::Device(Device &&other) noexcept
    : m_device(std::exchange(other.m_device, nullptr)), m_graphics_card(std::exchange(other.m_graphics_card, nullptr)),
      m_enable_vulkan_debug_markers(other.m_enable_vulkan_debug_markers), m_surface(other.m_surface) {}

Device::~Device() {
    if (m_allocator != nullptr) {
        vmaDestroyAllocator(m_allocator);
    }

    if (m_device != nullptr) {
        vkDestroyDevice(m_device, nullptr);
    }
}

void Device::set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type,
                                   const std::string &name) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(object);
    assert(!name.empty());
    assert(m_vk_debug_marker_set_object_name);

    auto name_info = make_info<VkDebugMarkerObjectNameInfoEXT>();
    name_info.objectType = object_type;
    name_info.object = reinterpret_cast<std::uint64_t>(object);
    name_info.pObjectName = name.c_str();

    if (const auto result = m_vk_debug_marker_set_object_name(m_device, &name_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker name " + name + "!", result);
    }
#endif
}

void Device::set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, const std::uint64_t name,
                                         const std::size_t memory_size, const void *memory_block) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(name);
    assert(memory_size > 0);
    assert(memory_block);
    assert(m_vk_debug_marker_set_object_tag);

    auto tag_info = make_info<VkDebugMarkerObjectTagInfoEXT>();
    tag_info.object = reinterpret_cast<std::uint64_t>(object);
    tag_info.objectType = object_type;
    tag_info.tagName = name;
    tag_info.tagSize = memory_size;
    tag_info.pTag = memory_block;

    if (const auto result = m_vk_debug_marker_set_object_tag(m_device, &tag_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker memory block!", result);
    }
#endif
}

void Device::bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                               const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());
    assert(m_vk_cmd_debug_marker_begin);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_begin(command_buffer, &debug_marker);
#endif
}

void Device::insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                                 const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());
    assert(m_vk_cmd_debug_marker_insert);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_insert(command_buffer, &debug_marker);
#endif
}

void Device::end_debug_region(const VkCommandBuffer command_buffer) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(m_vk_cmd_debug_marker_end);
    m_vk_cmd_debug_marker_end(command_buffer);
#endif
}

} // namespace inexor::vulkan_renderer::wrapper
