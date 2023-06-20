#include "inexor/vulkan-renderer/vk_tools/gpu_info.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <cstdint>

namespace inexor::vulkan_renderer::vk_tools {

void print_driver_vulkan_version() {
    std::uint32_t api_version = 0;

    if (const auto result = vkEnumerateInstanceVersion(&api_version); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceVersion returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Supported Vulkan API version: {}.{}.{}", VK_API_VERSION_MAJOR(api_version),
                  VK_API_VERSION_MINOR(api_version), VK_API_VERSION_PATCH(api_version));
}

void print_physical_device_queue_families(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    // Query how many physical device queue families are available
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    spdlog::trace("Number of queue families: {}", queue_family_count);

    if (queue_family_count == 0) {
        return;
    }

    // Store all available physical device queue family properties
    std::vector<VkQueueFamilyProperties> properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, properties.data());

    constexpr std::array QUEUE_BITS{VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT,
                                    VK_QUEUE_SPARSE_BINDING_BIT, VK_QUEUE_PROTECTED_BIT};

    for (std::size_t i = 0; i < queue_family_count; i++) {
        spdlog::trace("Queue family: {}", i);
        spdlog::trace("Queue count: {}", properties[i].queueCount);
        spdlog::trace("Timestamp valid bits: {}", properties[i].timestampValidBits);

        for (const auto &queue_bit : QUEUE_BITS) {
            if (static_cast<bool>(properties[i].queueFlags & queue_bit)) {
                spdlog::trace("{}", vk_tools::as_string(queue_bit));
            }
        }

        const auto granularity = properties[i].minImageTransferGranularity;

        spdlog::trace("Min image transfer granularity: width {}, height {}, depth {}", granularity.width,
                      granularity.height, granularity.depth);
    }
}

void print_instance_layers() {
    // Query how many instance layers are available
    std::uint32_t instance_layer_count = 0;
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceLayerProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Number of instance layers: {}", instance_layer_count);

    if (instance_layer_count == 0) {
        // This is not an error. Some platforms simply don't have any instance layers.
        return;
    }

    // Store all available instance layers
    std::vector<VkLayerProperties> instance_layers(instance_layer_count);
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceLayerProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &layer : instance_layers) {
        spdlog::trace("Name: {}", layer.layerName);
        spdlog::trace("Spec Version: {}", VK_API_VERSION_MAJOR(layer.specVersion),
                      VK_API_VERSION_MINOR(layer.specVersion), VK_API_VERSION_PATCH(layer.specVersion));
        spdlog::trace("Impl Version: {}", layer.implementationVersion);
        spdlog::trace("Description: {}", layer.description);
    }
}

void print_instance_extensions() {
    // Query how many instance extensions are available
    std::uint32_t instance_extension_count = 0;
    if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Number of instance extensions: {} ", instance_extension_count);

    if (instance_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any instance extensions.
        return;
    }

    // Store all available instance extensions
    std::vector<VkExtensionProperties> extensions(instance_extension_count);
    if (const auto result =
            vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, extensions.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &extension : extensions) {
        spdlog::trace("Spec version: {}.{}.{}\t Name: {}", VK_API_VERSION_MAJOR(extension.specVersion),
                      VK_API_VERSION_MINOR(extension.specVersion), VK_API_VERSION_PATCH(extension.specVersion),
                      extension.extensionName);
    }
}

void print_device_extensions(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    // First check how many device extensions are available
    std::uint32_t device_extension_count = 0;
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateDeviceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Number of device extensions: {}", device_extension_count);

    if (device_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any device extensions.
        return;
    }

    // Store all available device extensions
    std::vector<VkExtensionProperties> device_extensions(device_extension_count);
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count,
                                                                 device_extensions.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateDeviceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &extension : device_extensions) {
        spdlog::trace("Spec version: {}.{}.{}\t Name: {}", VK_API_VERSION_MAJOR(extension.specVersion),
                      VK_API_VERSION_MINOR(extension.specVersion), VK_API_VERSION_PATCH(extension.specVersion),
                      extension.extensionName);
    }
}

void print_surface_capabilities(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) {
    assert(physical_device);
    assert(surface);

    spdlog::trace("Printing surface capabilities");

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("minImageCount: {}", surface_capabilities.minImageCount);
    spdlog::trace("maxImageCount: {}", surface_capabilities.maxImageCount);
    spdlog::trace("currentExtent.width: {}", surface_capabilities.currentExtent.width);
    spdlog::trace("currentExtent.height: {}", surface_capabilities.currentExtent.height);
    spdlog::trace("minImageExtent.width: {}", surface_capabilities.minImageExtent.width);
    spdlog::trace("minImageExtent.height: {}", surface_capabilities.minImageExtent.height);
    spdlog::trace("maxImageExtent.width: {}", surface_capabilities.maxImageExtent.width);
    spdlog::trace("maxImageExtent.height: {}", surface_capabilities.maxImageExtent.height);
    spdlog::trace("maxImageArrayLayers: {}", surface_capabilities.maxImageArrayLayers);
    spdlog::trace("supportedTransforms: {}", surface_capabilities.supportedTransforms);
    spdlog::trace("currentTransform: {}", surface_capabilities.currentTransform);
    spdlog::trace("supportedCompositeAlpha: {}", surface_capabilities.supportedCompositeAlpha);
    spdlog::trace("supportedUsageFlags: {}", surface_capabilities.supportedUsageFlags);
}

void print_supported_surface_formats(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) {
    assert(physical_device);
    assert(surface);

    // First query how many physical device surface formats are available
    std::uint32_t format_count = 0;
    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceFormatsKHR returned {}!", result);
        return;
    }

    spdlog::trace("Supported surface formats: {}", format_count);

    if (format_count == 0) {
        return;
    }

    // Store all physical device surface formats are available
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    if (const auto result =
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceFormatsKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &format : surface_formats) {
        spdlog::trace("Surface format: {}", vk_tools::as_string(format.format));
    }
}

void print_presentation_modes(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) {
    assert(physical_device);
    assert(surface);

    // First query how many presentation modes are available
    std::uint32_t present_mode_count = 0;
    if (const auto result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfacePresentModesKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Available present modes: ", present_mode_count);

    if (present_mode_count == 0) {
        return;
    }

    // Store all available present modes
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                                      present_modes.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfacePresentModesKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &mode : present_modes) {
        spdlog::trace("Present mode: {}", vk_tools::as_string(mode));
    }
}

void print_physical_device_info(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    spdlog::trace("Physical Device: {}", properties.deviceName);

    spdlog::trace("Vulkan API supported version: {}.{}.{}", VK_API_VERSION_MAJOR(properties.apiVersion),
                  VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));

    // The driver version format is not standardised. It's not even always the same for one vendor!
    spdlog::trace("Vulkan API supported version: {}.{}.{}", VK_API_VERSION_MAJOR(properties.driverVersion),
                  VK_API_VERSION_MINOR(properties.driverVersion), VK_API_VERSION_PATCH(properties.driverVersion));
    spdlog::trace("Vendor ID: {}", properties.vendorID);
    spdlog::trace("Device ID: {}", properties.deviceID);
    spdlog::trace("Device type: {}", vk_tools::as_string(properties.deviceType));
}

void print_physical_device_memory_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    spdlog::trace("Physical device memory properties:");

    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    spdlog::trace("Number of memory types: {}", properties.memoryTypeCount);
    spdlog::trace("Number of heap types: {}", properties.memoryHeapCount);

    constexpr std::array MEM_PROP_FLAGS{
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,       VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,    VK_MEMORY_PROPERTY_PROTECTED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD, VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD};

    for (std::size_t i = 0; i < properties.memoryTypeCount; i++) {
        spdlog::trace("[{}] Heap index: {}", i, properties.memoryTypes[i].heapIndex);

        for (const auto &mem_prop_flag : MEM_PROP_FLAGS) {
            if (static_cast<bool>(properties.memoryTypes[i].propertyFlags & mem_prop_flag)) {
                spdlog::trace("{}", vk_tools::as_string(mem_prop_flag));
            }
        }
    }

    constexpr std::array MEM_HEAP_PROP_FLAGS{VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
                                             VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR, VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM};

    for (std::size_t i = 0; i < properties.memoryHeapCount; i++) {
        spdlog::trace("Heap [{}], memory size: {}", i, properties.memoryHeaps[i].size / (1000 * 1000));

        for (const auto &mem_heap_prop_flag : MEM_HEAP_PROP_FLAGS) {
            if (static_cast<bool>(properties.memoryHeaps[i].flags & mem_heap_prop_flag)) {
                spdlog::trace("{}", vk_tools::as_string(mem_heap_prop_flag));
            }
        }
    }
}

void print_physical_device_features(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    spdlog::trace("Physical device features:");

    spdlog::trace("robustBufferAccess: {}", features.robustBufferAccess);
    spdlog::trace("fullDrawIndexUint32: {}", features.fullDrawIndexUint32);
    spdlog::trace("imageCubeArray: {}", features.imageCubeArray);
    spdlog::trace("independentBlend: {}", features.independentBlend);
    spdlog::trace("geometryShader: {}", features.geometryShader);
    spdlog::trace("tessellationShader: {}", features.tessellationShader);
    spdlog::trace("sampleRateShading: {}", features.sampleRateShading);
    spdlog::trace("dualSrcBlend: {}", features.dualSrcBlend);
    spdlog::trace("logicOp: {}", features.logicOp);
    spdlog::trace("multiDrawIndirect: {}", features.multiDrawIndirect);
    spdlog::trace("drawIndirectFirstInstance: {}", features.drawIndirectFirstInstance);
    spdlog::trace("depthClamp: {}", features.depthClamp);
    spdlog::trace("depthBiasClamp: {}", features.depthBiasClamp);
    spdlog::trace("fillModeNonSolid: {}", features.fillModeNonSolid);
    spdlog::trace("depthBounds: {}", features.depthBounds);
    spdlog::trace("wideLines: {}", features.wideLines);
    spdlog::trace("largePoints: {}", features.largePoints);
    spdlog::trace("alphaToOne: {}", features.alphaToOne);
    spdlog::trace("multiViewport: {}", features.multiViewport);
    spdlog::trace("samplerAnisotropy: {}", features.samplerAnisotropy);
    spdlog::trace("textureCompressionETC2: {}", features.textureCompressionETC2);
    spdlog::trace("textureCompressionASTC_LDR: {}", features.textureCompressionASTC_LDR);
    spdlog::trace("textureCompressionBC: {}", features.textureCompressionBC);
    spdlog::trace("occlusionQueryPrecise: {}", features.occlusionQueryPrecise);
    spdlog::trace("pipelineStatisticsQuery: {}", features.pipelineStatisticsQuery);
    spdlog::trace("vertexPipelineStoresAndAtomics: {}", features.vertexPipelineStoresAndAtomics);
    spdlog::trace("fragmentStoresAndAtomics: {}", features.fragmentStoresAndAtomics);
    spdlog::trace("shaderTessellationAndGeometryPointSize: {}", features.shaderTessellationAndGeometryPointSize);
    spdlog::trace("shaderImageGatherExtended: {}", features.shaderImageGatherExtended);
    spdlog::trace("shaderStorageImageExtendedFormats: {}", features.shaderStorageImageExtendedFormats);
    spdlog::trace("shaderStorageImageMultisample: {}", features.shaderStorageImageMultisample);
    spdlog::trace("shaderStorageImageReadWithoutFormat: {}", features.shaderStorageImageReadWithoutFormat);
    spdlog::trace("shaderStorageImageWriteWithoutFormat: {}", features.shaderStorageImageWriteWithoutFormat);
    spdlog::trace("shaderUniformBufferArrayDynamicIndexing: {}", features.shaderUniformBufferArrayDynamicIndexing);
    spdlog::trace("shaderSampledImageArrayDynamicIndexing: {}", features.shaderSampledImageArrayDynamicIndexing);
    spdlog::trace("shaderStorageBufferArrayDynamicIndexing: {}", features.shaderStorageBufferArrayDynamicIndexing);
    spdlog::trace("shaderStorageImageArrayDynamicIndexing: {}", features.shaderStorageImageArrayDynamicIndexing);
    spdlog::trace("shaderClipDistance: {}", features.shaderClipDistance);
    spdlog::trace("shaderCullDistance: {}", features.shaderCullDistance);
    spdlog::trace("shaderFloat64: {}", features.shaderFloat64);
    spdlog::trace("shaderInt64: {}", features.shaderInt64);
    spdlog::trace("shaderInt16: {}", features.shaderInt16);
    spdlog::trace("shaderResourceResidency: {}", features.shaderResourceResidency);
    spdlog::trace("shaderResourceMinLod: {}", features.shaderResourceMinLod);
    spdlog::trace("sparseBinding: {}", features.sparseBinding);
    spdlog::trace("sparseResidencyBuffer: {}", features.sparseResidencyBuffer);
    spdlog::trace("sparseResidencyImage2D: {}", features.sparseResidencyImage2D);
    spdlog::trace("sparseResidencyImage3D: {}", features.sparseResidencyImage3D);
    spdlog::trace("sparseResidency2Samples: {}", features.sparseResidency2Samples);
    spdlog::trace("sparseResidency4Samples: {}", features.sparseResidency4Samples);
    spdlog::trace("sparseResidency8Samples: {}", features.sparseResidency8Samples);
    spdlog::trace("sparseResidency16Samples: {}", features.sparseResidency16Samples);
    spdlog::trace("sparseResidencyAliased: {}", features.sparseResidencyAliased);
    spdlog::trace("variableMultisampleRate: {}", features.variableMultisampleRate);
    spdlog::trace("inheritedQueries: {}", features.inheritedQueries);
}

void print_physical_device_sparse_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    spdlog::trace("Physical device sparse properties:");

    const auto props = properties.sparseProperties;

    spdlog::trace("residencyStandard2DBlockShape: {}", props.residencyStandard2DBlockShape);
    spdlog::trace("residencyStandard2DMultisampleBlockShape: {}", props.residencyStandard2DMultisampleBlockShape);
    spdlog::trace("residencyStandard3DBlockShape: {}", props.residencyStandard3DBlockShape);
    spdlog::trace("residencyAlignedMipSize: {}", props.residencyAlignedMipSize);
    spdlog::trace("residencyNonResidentStrict: {}", props.residencyNonResidentStrict);
}

void print_physical_device_limits(const VkPhysicalDevice physical_device) {
    assert(physical_device);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    spdlog::trace("Gpu limits:");

    const auto limits = properties.limits;

    spdlog::trace("maxImageDimension1D: {}", limits.maxImageDimension1D);
    spdlog::trace("maxImageDimension2D: {}", limits.maxImageDimension2D);
    spdlog::trace("maxImageDimension3D: {}", limits.maxImageDimension3D);
    spdlog::trace("maxImageDimensionCube: {}", limits.maxImageDimensionCube);
    spdlog::trace("maxImageArrayLayers: {}", limits.maxImageArrayLayers);
    spdlog::trace("maxTexelBufferElements: {}", limits.maxTexelBufferElements);
    spdlog::trace("maxUniformBufferRange: {}", limits.maxUniformBufferRange);
    spdlog::trace("maxStorageBufferRange: {}", limits.maxStorageBufferRange);
    spdlog::trace("maxPushConstantsSize: {}", limits.maxPushConstantsSize);
    spdlog::trace("maxMemoryAllocationCount: {}", limits.maxMemoryAllocationCount);
    spdlog::trace("maxSamplerAllocationCount: {}", limits.maxSamplerAllocationCount);
    spdlog::trace("bufferImageGranularity: {}", limits.bufferImageGranularity);
    spdlog::trace("sparseAddressSpaceSize: {}", limits.sparseAddressSpaceSize);
    spdlog::trace("maxBoundDescriptorSets: {}", limits.maxBoundDescriptorSets);
    spdlog::trace("maxPerStageDescriptorSamplers: {}", limits.maxPerStageDescriptorSamplers);
    spdlog::trace("maxPerStageDescriptorUniformBuffers: {}", limits.maxPerStageDescriptorUniformBuffers);
    spdlog::trace("maxPerStageDescriptorStorageBuffers: {}", limits.maxPerStageDescriptorStorageBuffers);
    spdlog::trace("maxPerStageDescriptorSampledImages: {}", limits.maxPerStageDescriptorSampledImages);
    spdlog::trace("maxPerStageDescriptorStorageImages: {}", limits.maxPerStageDescriptorStorageImages);
    spdlog::trace("maxPerStageDescriptorInputAttachments: {}", limits.maxPerStageDescriptorInputAttachments);
    spdlog::trace("maxPerStageResources: {}", limits.maxPerStageResources);
    spdlog::trace("maxDescriptorSetSamplers: {}", limits.maxDescriptorSetSamplers);
    spdlog::trace("maxDescriptorSetUniformBuffers: {}", limits.maxDescriptorSetUniformBuffers);
    spdlog::trace("maxDescriptorSetUniformBuffersDynamic: {}", limits.maxDescriptorSetUniformBuffersDynamic);
    spdlog::trace("maxDescriptorSetStorageBuffers: {}", limits.maxDescriptorSetStorageBuffers);
    spdlog::trace("maxDescriptorSetStorageBuffersDynamic: {}", limits.maxDescriptorSetStorageBuffersDynamic);
    spdlog::trace("maxDescriptorSetSampledImages: {}", limits.maxDescriptorSetSampledImages);
    spdlog::trace("maxDescriptorSetStorageImages: {}", limits.maxDescriptorSetStorageImages);
    spdlog::trace("maxDescriptorSetInputAttachments: {}", limits.maxDescriptorSetInputAttachments);
    spdlog::trace("maxVertexInputAttributes: {}", limits.maxVertexInputAttributes);
    spdlog::trace("maxVertexInputBindings: {}", limits.maxVertexInputBindings);
    spdlog::trace("maxVertexInputAttributeOffset: {}", limits.maxVertexInputAttributeOffset);
    spdlog::trace("maxVertexInputBindingStride: {}", limits.maxVertexInputBindingStride);
    spdlog::trace("maxVertexOutputComponents: {}", limits.maxVertexOutputComponents);
    spdlog::trace("maxTessellationGenerationLevel: {}", limits.maxTessellationGenerationLevel);
    spdlog::trace("maxTessellationPatchSize: {}", limits.maxTessellationPatchSize);
    spdlog::trace("maxTessellationControlPerVertexInputComponents: {}",
                  limits.maxTessellationControlPerVertexInputComponents);
    spdlog::trace("maxTessellationControlPerVertexOutputComponents: {}",
                  limits.maxTessellationControlPerVertexOutputComponents);
    spdlog::trace("maxTessellationControlPerPatchOutputComponents: {}",
                  limits.maxTessellationControlPerPatchOutputComponents);
    spdlog::trace("maxTessellationControlTotalOutputComponents: {}",
                  limits.maxTessellationControlTotalOutputComponents);
    spdlog::trace("maxTessellationEvaluationInputComponents: {}", limits.maxTessellationEvaluationInputComponents);
    spdlog::trace("maxTessellationEvaluationOutputComponents: {}", limits.maxTessellationEvaluationOutputComponents);
    spdlog::trace("maxGeometryShaderInvocations: {}", limits.maxGeometryShaderInvocations);
    spdlog::trace("maxGeometryInputComponents: {}", limits.maxGeometryInputComponents);
    spdlog::trace("maxGeometryOutputComponents: {}", limits.maxGeometryOutputComponents);
    spdlog::trace("maxGeometryOutputVertices: {}", limits.maxGeometryOutputVertices);
    spdlog::trace("maxGeometryTotalOutputComponents: {}", limits.maxGeometryTotalOutputComponents);
    spdlog::trace("maxFragmentInputComponents: {}", limits.maxFragmentInputComponents);
    spdlog::trace("maxFragmentOutputAttachments: {}", limits.maxFragmentOutputAttachments);
    spdlog::trace("maxFragmentDualSrcAttachments: {}", limits.maxFragmentDualSrcAttachments);
    spdlog::trace("maxFragmentCombinedOutputResources: {}", limits.maxFragmentCombinedOutputResources);
    spdlog::trace("maxComputeSharedMemorySize: {}", limits.maxComputeSharedMemorySize);
    spdlog::trace("maxComputeWorkGroupCount[0]: {}", limits.maxComputeWorkGroupCount[0]);
    spdlog::trace("maxComputeWorkGroupCount[1]: {}", limits.maxComputeWorkGroupCount[1]);
    spdlog::trace("maxComputeWorkGroupCount[2]: {}", limits.maxComputeWorkGroupCount[2]);
    spdlog::trace("maxComputeWorkGroupInvocations: {}", limits.maxComputeWorkGroupInvocations);
    spdlog::trace("maxComputeWorkGroupSize[0]: {}", limits.maxComputeWorkGroupSize[0]);
    spdlog::trace("maxComputeWorkGroupSize[1]: {}", limits.maxComputeWorkGroupSize[1]);
    spdlog::trace("maxComputeWorkGroupSize[2]: {}", limits.maxComputeWorkGroupSize[2]);
    spdlog::trace("subPixelPrecisionBits: {}", limits.subPixelPrecisionBits);
    spdlog::trace("subTexelPrecisionBits: {}", limits.subTexelPrecisionBits);
    spdlog::trace("mipmapPrecisionBits: {}", limits.mipmapPrecisionBits);
    spdlog::trace("maxDrawIndexedIndexValue: {}", limits.maxDrawIndexedIndexValue);
    spdlog::trace("maxDrawIndirectCount: {}", limits.maxDrawIndirectCount);
    spdlog::trace("maxSamplerLodBias: {}", limits.maxSamplerLodBias);
    spdlog::trace("maxSamplerAnisotropy: {}", limits.maxSamplerAnisotropy);
    spdlog::trace("maxViewports: {}", limits.maxViewports);
    spdlog::trace("maxViewportDimensions[0]: {}", limits.maxViewportDimensions[0]);
    spdlog::trace("maxViewportDimensions[1]: {}", limits.maxViewportDimensions[1]);
    spdlog::trace("viewportBoundsRange[0]: {}", limits.viewportBoundsRange[0]);
    spdlog::trace("viewportBoundsRange[1]: {}", limits.viewportBoundsRange[1]);
    spdlog::trace("viewportSubPixelBits: {}", limits.viewportSubPixelBits);
    spdlog::trace("minMemoryMapAlignment: {}", limits.minMemoryMapAlignment);
    spdlog::trace("minTexelBufferOffsetAlignment: {}", limits.minTexelBufferOffsetAlignment);
    spdlog::trace("minUniformBufferOffsetAlignment: {}", limits.minUniformBufferOffsetAlignment);
    spdlog::trace("minStorageBufferOffsetAlignment: {}", limits.minStorageBufferOffsetAlignment);
    spdlog::trace("minTexelOffset: {}", limits.minTexelOffset);
    spdlog::trace("maxTexelOffset: {}", limits.maxTexelOffset);
    spdlog::trace("minTexelGatherOffset: {}", limits.minTexelGatherOffset);
    spdlog::trace("maxTexelGatherOffset: {}", limits.maxTexelGatherOffset);
    spdlog::trace("minInterpolationOffset: {}", limits.minInterpolationOffset);
    spdlog::trace("maxInterpolationOffset: {}", limits.maxInterpolationOffset);
    spdlog::trace("subPixelInterpolationOffsetBits: {}", limits.subPixelInterpolationOffsetBits);
    spdlog::trace("maxFramebufferWidth: {}", limits.maxFramebufferWidth);
    spdlog::trace("maxFramebufferHeight: {}", limits.maxFramebufferHeight);
    spdlog::trace("maxFramebufferLayers: {}", limits.maxFramebufferLayers);
    spdlog::trace("framebufferColorSampleCounts: {}", limits.framebufferColorSampleCounts);
    spdlog::trace("framebufferDepthSampleCounts: {}", limits.framebufferDepthSampleCounts);
    spdlog::trace("framebufferStencilSampleCounts: {}", limits.framebufferStencilSampleCounts);
    spdlog::trace("framebufferNoAttachmentsSampleCounts: {}", limits.framebufferNoAttachmentsSampleCounts);
    spdlog::trace("maxColorAttachments: {}", limits.maxColorAttachments);
    spdlog::trace("sampledImageColorSampleCounts: {}", limits.sampledImageColorSampleCounts);
    spdlog::trace("sampledImageIntegerSampleCounts: {}", limits.sampledImageIntegerSampleCounts);
    spdlog::trace("sampledImageDepthSampleCounts: {}", limits.sampledImageDepthSampleCounts);
    spdlog::trace("sampledImageStencilSampleCounts: {}", limits.sampledImageStencilSampleCounts);
    spdlog::trace("storageImageSampleCounts: {}", limits.storageImageSampleCounts);
    spdlog::trace("maxSampleMaskWords: {}", limits.maxSampleMaskWords);
    spdlog::trace("timestampComputeAndGraphics: {}", limits.timestampComputeAndGraphics);
    spdlog::trace("timestampPeriod: {}", limits.timestampPeriod);
    spdlog::trace("maxClipDistances: {}", limits.maxClipDistances);
    spdlog::trace("maxCullDistances: {}", limits.maxCullDistances);
    spdlog::trace("maxCombinedClipAndCullDistances: {}", limits.maxCombinedClipAndCullDistances);
    spdlog::trace("discreteQueuePriorities: {}", limits.discreteQueuePriorities);
    spdlog::trace("pointSizeRange[0]: {}", limits.pointSizeRange[0]);
    spdlog::trace("pointSizeRange[1]: {}", limits.pointSizeRange[1]);
    spdlog::trace("lineWidthRange[0]: {}", limits.lineWidthRange[0]);
    spdlog::trace("lineWidthRange[1]: {}", limits.lineWidthRange[1]);
    spdlog::trace("pointSizeGranularity: {}", limits.pointSizeGranularity);
    spdlog::trace("lineWidthGranularity: {}", limits.lineWidthGranularity);
    spdlog::trace("strictLines: {}", limits.strictLines);
    spdlog::trace("standardSampleLocations: {}", limits.standardSampleLocations);
    spdlog::trace("optimalBufferCopyOffsetAlignment: {}", limits.optimalBufferCopyOffsetAlignment);
    spdlog::trace("optimalBufferCopyRowPitchAlignment: {}", limits.optimalBufferCopyRowPitchAlignment);
    spdlog::trace("nonCoherentAtomSize: {}", limits.nonCoherentAtomSize);
}

void print_all_physical_devices(const VkInstance instance, const VkSurfaceKHR surface) {
    assert(instance);
    assert(surface);

    // First query how many physical devices are available
    std::uint32_t count = 0;
    if (const auto result = vkEnumeratePhysicalDevices(instance, &count, nullptr); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumeratePhysicalDevices returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::trace("Number of available physical devices: {}", count);

    if (count == 0) {
        return;
    }

    // Store all available physical devices
    std::vector<VkPhysicalDevice> physical_devices(count);
    if (const auto result = vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumeratePhysicalDevices returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &physical_device : physical_devices) {
        print_device_extensions(physical_device);
        print_physical_device_info(physical_device);
        print_physical_device_queue_families(physical_device);
        print_surface_capabilities(physical_device, surface);
        print_supported_surface_formats(physical_device, surface);
        print_presentation_modes(physical_device, surface);
        print_physical_device_memory_properties(physical_device);
        print_physical_device_features(physical_device);
        print_physical_device_sparse_properties(physical_device);
        print_physical_device_limits(physical_device);
    }
}

} // namespace inexor::vulkan_renderer::vk_tools
