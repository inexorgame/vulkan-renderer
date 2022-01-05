#include "inexor/vulkan-renderer/vk_tools/gpu_info.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <cstdint>

namespace inexor::vulkan_renderer::vk_tools {

void print_driver_vulkan_version() {
    std::uint32_t api_version{0};

    if (const auto result = vkEnumerateInstanceVersion(&api_version); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceVersion returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Supported Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(api_version),
                  VK_VERSION_MINOR(api_version), VK_VERSION_PATCH(api_version));
}

void print_physical_device_queue_families(const VkPhysicalDevice gpu) {
    assert(gpu);

    std::uint32_t queue_family_count{0};

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);

    spdlog::debug("Number of queue families: {}", queue_family_count);

    if (queue_family_count == 0) {
        return;
    }

    std::vector<VkQueueFamilyProperties> queue_family_properties;
    queue_family_properties.reserve(queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_properties.data());

    constexpr std::array QUEUE_BITS{VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT,
                                    VK_QUEUE_SPARSE_BINDING_BIT, VK_QUEUE_PROTECTED_BIT};

    for (std::size_t i = 0; i < queue_family_count; i++) {
        spdlog::debug("Queue family: {}", i);
        spdlog::debug("Queue count: {}", queue_family_properties[i].queueCount);
        spdlog::debug("Timestamp valid bits: {}", queue_family_properties[i].timestampValidBits);

        for (const auto &queue_bit : QUEUE_BITS) {
            if (static_cast<bool>(queue_family_properties[i].queueFlags & queue_bit)) {
                spdlog::debug("{}", vk_tools::as_string(queue_bit));
            }
        }

        spdlog::debug("Min image transfer granularity: width {}, height {}, depth {}",
                      queue_family_properties[i].minImageTransferGranularity.width,
                      queue_family_properties[i].minImageTransferGranularity.height,
                      queue_family_properties[i].minImageTransferGranularity.depth);
    }
}

void print_instance_layers() {
    std::uint32_t instance_layer_count{0};

    // Query how many instance layers are available.
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceLayerProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Number of instance layers: {}", instance_layer_count);

    if (instance_layer_count == 0) {
        // This is not an error. Some platforms simply don't have any instance layers.
        return;
    }

    std::vector<VkLayerProperties> instance_layers;
    instance_layers.reserve(instance_layer_count);

    // Store all available instance layers.
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceLayerProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &layer : instance_layers) {
        spdlog::debug("Name: {}", layer.layerName);
        spdlog::debug("Spec Version: {}", VK_VERSION_MAJOR(layer.specVersion), VK_VERSION_MINOR(layer.specVersion),
                      VK_VERSION_PATCH(layer.specVersion));
        spdlog::debug("Impl Version: {}", layer.implementationVersion);
        spdlog::debug("Description: {}", layer.description);
    }
}

void print_instance_extensions() {
    std::uint32_t instance_extension_count{0};

    // Query how many instance extensions are available.
    if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Number of instance extensions: {} ", instance_extension_count);

    if (instance_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any instance extensions.
        return;
    }

    std::vector<VkExtensionProperties> extensions;
    extensions.reserve(instance_extension_count);

    // Store all available instance extensions.
    if (const auto result =
            vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, extensions.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &extension : extensions) {
        spdlog::debug("Spec version: {}.{}.{}\t Name: {}", VK_VERSION_MAJOR(extension.specVersion),
                      VK_VERSION_MINOR(extension.specVersion), VK_VERSION_PATCH(extension.specVersion),
                      extension.extensionName);
    }
}

void print_device_extensions(const VkPhysicalDevice gpu) {
    assert(gpu);

    std::uint32_t device_extension_count{0};

    // First check how many device extensions are available.
    if (const auto result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateDeviceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Number of device extensions: {}", device_extension_count);

    if (device_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any device extensions.
        return;
    }

    std::vector<VkExtensionProperties> device_extensions;
    device_extensions.reserve(device_extension_count);

    // Store all available device extensions.
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(gpu, nullptr, &device_extension_count, device_extensions.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateDeviceExtensionProperties returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &extension : device_extensions) {
        spdlog::debug("Spec version: {}.{}.{}\t Name: {}", VK_VERSION_MAJOR(extension.specVersion),
                      VK_VERSION_MINOR(extension.specVersion), VK_VERSION_PATCH(extension.specVersion),
                      extension.extensionName);
    }
}

void print_surface_capabilities(const VkPhysicalDevice gpu, const VkSurfaceKHR surface) {
    assert(gpu);
    assert(surface);

    spdlog::debug("Printing surface capabilities.");

    VkSurfaceCapabilitiesKHR surface_capabilities;

    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("minImageCount: {}", surface_capabilities.minImageCount);
    spdlog::debug("maxImageCount: {}", surface_capabilities.maxImageCount);
    spdlog::debug("currentExtent.width: {}", surface_capabilities.currentExtent.width);
    spdlog::debug("currentExtent.height: {}", surface_capabilities.currentExtent.height);
    spdlog::debug("minImageExtent.width: {}", surface_capabilities.minImageExtent.width);
    spdlog::debug("minImageExtent.height: {}", surface_capabilities.minImageExtent.height);
    spdlog::debug("maxImageExtent.width: {}", surface_capabilities.maxImageExtent.width);
    spdlog::debug("maxImageExtent.height: {}", surface_capabilities.maxImageExtent.height);
    spdlog::debug("maxImageArrayLayers: {}", surface_capabilities.maxImageArrayLayers);
    spdlog::debug("supportedTransforms: {}", surface_capabilities.supportedTransforms);
    spdlog::debug("currentTransform: {}", surface_capabilities.currentTransform);
    spdlog::debug("supportedCompositeAlpha: {}", surface_capabilities.supportedCompositeAlpha);
    spdlog::debug("supportedUsageFlags: {}", surface_capabilities.supportedUsageFlags);
}

void print_supported_surface_formats(const VkPhysicalDevice gpu, const VkSurfaceKHR surface) {
    assert(gpu);
    assert(surface);

    std::uint32_t format_count{0};

    // Query how many formats are supported.
    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceFormatsKHR returned {}!", result);
        return;
    }

    spdlog::debug("Supported surface formats: {}", format_count);

    if (format_count == 0) {
        return;
    }

    std::vector<VkSurfaceFormatKHR> surface_formats;
    surface_formats.reserve(format_count);

    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surface_formats.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfaceFormatsKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &format : surface_formats) {
        spdlog::debug("Surface format: {}", vk_tools::as_string(format.format));
    }
}

void print_presentation_modes(const VkPhysicalDevice gpu, const VkSurfaceKHR surface) {
    assert(gpu);
    assert(surface);

    std::uint32_t present_mode_count{0};

    // Query how many presentation modes are available.
    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfacePresentModesKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Available present modes: ", present_mode_count);

    if (present_mode_count == 0) {
        return;
    }

    std::vector<VkPresentModeKHR> present_modes;
    present_modes.reserve(present_mode_count);

    if (const auto result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, present_modes.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkGetPhysicalDeviceSurfacePresentModesKHR returned {}!", vk_tools::as_string(result));
        return;
    }

    for (const auto &mode : present_modes) {
        spdlog::debug("Present mode: {}", vk_tools::as_string(mode));
    }
}

void print_physical_device_info(const VkPhysicalDevice gpu) {
    assert(gpu);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

    spdlog::debug("Gpu: {}", gpu_properties.deviceName);

    spdlog::debug("Vulkan API supported version: {}.{}.{}", VK_VERSION_MAJOR(gpu_properties.apiVersion),
                  VK_VERSION_MINOR(gpu_properties.apiVersion), VK_VERSION_PATCH(gpu_properties.apiVersion));

    // The driver version format is not standardised. It's not even always the same for one vendor!
    spdlog::debug("Vulkan API supported version: {}.{}.{}", VK_VERSION_MAJOR(gpu_properties.driverVersion),
                  VK_VERSION_MINOR(gpu_properties.driverVersion), VK_VERSION_PATCH(gpu_properties.driverVersion));
    spdlog::debug("Vendor ID: {}", gpu_properties.vendorID);
    spdlog::debug("Device ID: {}", gpu_properties.deviceID);
    spdlog::debug("Device type: {}", vk_tools::as_string(gpu_properties.deviceType));
}

void print_physical_device_memory_properties(const VkPhysicalDevice gpu) {
    assert(gpu);

    spdlog::debug("Gpu memory properties:");

    VkPhysicalDeviceMemoryProperties gpu_mem_properties;

    vkGetPhysicalDeviceMemoryProperties(gpu, &gpu_mem_properties);

    spdlog::debug("Number of memory types: {}", gpu_mem_properties.memoryTypeCount);
    spdlog::debug("Number of heap types: {}", gpu_mem_properties.memoryHeapCount);

    constexpr std::array MEM_PROP_FLAGS{
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,       VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,    VK_MEMORY_PROPERTY_PROTECTED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD, VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD};

    for (std::size_t i = 0; i < gpu_mem_properties.memoryTypeCount; i++) {
        spdlog::debug("[{}] Heap index: {}", i, gpu_mem_properties.memoryTypes[i].heapIndex);

        for (const auto &mem_prop_flag : MEM_PROP_FLAGS) {
            if (static_cast<bool>(gpu_mem_properties.memoryTypes[i].propertyFlags & mem_prop_flag)) {
                spdlog::debug("{}", vk_tools::as_string(mem_prop_flag));
            }
        }
    }

    constexpr std::array MEM_HEAP_PROP_FLAGS{VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
                                             VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR, VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM};

    for (std::size_t i = 0; i < gpu_mem_properties.memoryHeapCount; i++) {
        spdlog::debug("Heap [{}], memory size: {}", i, gpu_mem_properties.memoryHeaps[i].size / (1000 * 1000));

        for (const auto &mem_heap_prop_flag : MEM_HEAP_PROP_FLAGS) {
            if (static_cast<bool>(gpu_mem_properties.memoryHeaps[i].flags & mem_heap_prop_flag)) {
                spdlog::debug("{}", vk_tools::as_string(mem_heap_prop_flag));
            }
        }
    }
}

void print_physical_device_features(const VkPhysicalDevice gpu) {
    assert(gpu);

    VkPhysicalDeviceFeatures gpu_features;

    vkGetPhysicalDeviceFeatures(gpu, &gpu_features);

    spdlog::debug("Gpu features:");

    spdlog::debug("robustBufferAccess: {}", gpu_features.robustBufferAccess);
    spdlog::debug("fullDrawIndexUint32: {}", gpu_features.fullDrawIndexUint32);
    spdlog::debug("imageCubeArray: {}", gpu_features.imageCubeArray);
    spdlog::debug("independentBlend: {}", gpu_features.independentBlend);
    spdlog::debug("geometryShader: {}", gpu_features.geometryShader);
    spdlog::debug("tessellationShader: {}", gpu_features.tessellationShader);
    spdlog::debug("sampleRateShading: {}", gpu_features.sampleRateShading);
    spdlog::debug("dualSrcBlend: {}", gpu_features.dualSrcBlend);
    spdlog::debug("logicOp: {}", gpu_features.logicOp);
    spdlog::debug("multiDrawIndirect: {}", gpu_features.multiDrawIndirect);
    spdlog::debug("drawIndirectFirstInstance: {}", gpu_features.drawIndirectFirstInstance);
    spdlog::debug("depthClamp: {}", gpu_features.depthClamp);
    spdlog::debug("depthBiasClamp: {}", gpu_features.depthBiasClamp);
    spdlog::debug("fillModeNonSolid: {}", gpu_features.fillModeNonSolid);
    spdlog::debug("depthBounds: {}", gpu_features.depthBounds);
    spdlog::debug("wideLines: {}", gpu_features.wideLines);
    spdlog::debug("largePoints: {}", gpu_features.largePoints);
    spdlog::debug("alphaToOne: {}", gpu_features.alphaToOne);
    spdlog::debug("multiViewport: {}", gpu_features.multiViewport);
    spdlog::debug("samplerAnisotropy: {}", gpu_features.samplerAnisotropy);
    spdlog::debug("textureCompressionETC2: {}", gpu_features.textureCompressionETC2);
    spdlog::debug("textureCompressionASTC_LDR: {}", gpu_features.textureCompressionASTC_LDR);
    spdlog::debug("textureCompressionBC: {}", gpu_features.textureCompressionBC);
    spdlog::debug("occlusionQueryPrecise: {}", gpu_features.occlusionQueryPrecise);
    spdlog::debug("pipelineStatisticsQuery: {}", gpu_features.pipelineStatisticsQuery);
    spdlog::debug("vertexPipelineStoresAndAtomics: {}", gpu_features.vertexPipelineStoresAndAtomics);
    spdlog::debug("fragmentStoresAndAtomics: {}", gpu_features.fragmentStoresAndAtomics);
    spdlog::debug("shaderTessellationAndGeometryPointSize: {}", gpu_features.shaderTessellationAndGeometryPointSize);
    spdlog::debug("shaderImageGatherExtended: {}", gpu_features.shaderImageGatherExtended);
    spdlog::debug("shaderStorageImageExtendedFormats: {}", gpu_features.shaderStorageImageExtendedFormats);
    spdlog::debug("shaderStorageImageMultisample: {}", gpu_features.shaderStorageImageMultisample);
    spdlog::debug("shaderStorageImageReadWithoutFormat: {}", gpu_features.shaderStorageImageReadWithoutFormat);
    spdlog::debug("shaderStorageImageWriteWithoutFormat: {}", gpu_features.shaderStorageImageWriteWithoutFormat);
    spdlog::debug("shaderUniformBufferArrayDynamicIndexing: {}", gpu_features.shaderUniformBufferArrayDynamicIndexing);
    spdlog::debug("shaderSampledImageArrayDynamicIndexing: {}", gpu_features.shaderSampledImageArrayDynamicIndexing);
    spdlog::debug("shaderStorageBufferArrayDynamicIndexing: {}", gpu_features.shaderStorageBufferArrayDynamicIndexing);
    spdlog::debug("shaderStorageImageArrayDynamicIndexing: {}", gpu_features.shaderStorageImageArrayDynamicIndexing);
    spdlog::debug("shaderClipDistance: {}", gpu_features.shaderClipDistance);
    spdlog::debug("shaderCullDistance: {}", gpu_features.shaderCullDistance);
    spdlog::debug("shaderFloat64: {}", gpu_features.shaderFloat64);
    spdlog::debug("shaderInt64: {}", gpu_features.shaderInt64);
    spdlog::debug("shaderInt16: {}", gpu_features.shaderInt16);
    spdlog::debug("shaderResourceResidency: {}", gpu_features.shaderResourceResidency);
    spdlog::debug("shaderResourceMinLod: {}", gpu_features.shaderResourceMinLod);
    spdlog::debug("sparseBinding: {}", gpu_features.sparseBinding);
    spdlog::debug("sparseResidencyBuffer: {}", gpu_features.sparseResidencyBuffer);
    spdlog::debug("sparseResidencyImage2D: {}", gpu_features.sparseResidencyImage2D);
    spdlog::debug("sparseResidencyImage3D: {}", gpu_features.sparseResidencyImage3D);
    spdlog::debug("sparseResidency2Samples: {}", gpu_features.sparseResidency2Samples);
    spdlog::debug("sparseResidency4Samples: {}", gpu_features.sparseResidency4Samples);
    spdlog::debug("sparseResidency8Samples: {}", gpu_features.sparseResidency8Samples);
    spdlog::debug("sparseResidency16Samples: {}", gpu_features.sparseResidency16Samples);
    spdlog::debug("sparseResidencyAliased: {}", gpu_features.sparseResidencyAliased);
    spdlog::debug("variableMultisampleRate: {}", gpu_features.variableMultisampleRate);
    spdlog::debug("inheritedQueries: {}", gpu_features.inheritedQueries);
}

void print_physical_device_sparse_properties(const VkPhysicalDevice gpu) {
    assert(gpu);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

    spdlog::debug("Gpu sparse properties:");

    const auto props = gpu_properties.sparseProperties;

    spdlog::debug("residencyStandard2DBlockShape: {}", props.residencyStandard2DBlockShape);
    spdlog::debug("residencyStandard2DMultisampleBlockShape: {}", props.residencyStandard2DMultisampleBlockShape);
    spdlog::debug("residencyStandard3DBlockShape: {}", props.residencyStandard3DBlockShape);
    spdlog::debug("residencyAlignedMipSize: {}", props.residencyAlignedMipSize);
    spdlog::debug("residencyNonResidentStrict: {}", props.residencyNonResidentStrict);
}

void print_physical_device_limits(const VkPhysicalDevice gpu) {
    assert(gpu);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

    spdlog::debug("Gpu limits:");

    const auto limits = gpu_properties.limits;

    spdlog::debug("maxImageDimension1D: {}", limits.maxImageDimension1D);
    spdlog::debug("maxImageDimension2D: {}", limits.maxImageDimension2D);
    spdlog::debug("maxImageDimension3D: {}", limits.maxImageDimension3D);
    spdlog::debug("maxImageDimensionCube: {}", limits.maxImageDimensionCube);
    spdlog::debug("maxImageArrayLayers: {}", limits.maxImageArrayLayers);
    spdlog::debug("maxTexelBufferElements: {}", limits.maxTexelBufferElements);
    spdlog::debug("maxUniformBufferRange: {}", limits.maxUniformBufferRange);
    spdlog::debug("maxStorageBufferRange: {}", limits.maxStorageBufferRange);
    spdlog::debug("maxPushConstantsSize: {}", limits.maxPushConstantsSize);
    spdlog::debug("maxMemoryAllocationCount: {}", limits.maxMemoryAllocationCount);
    spdlog::debug("maxSamplerAllocationCount: {}", limits.maxSamplerAllocationCount);
    spdlog::debug("bufferImageGranularity: {}", limits.bufferImageGranularity);
    spdlog::debug("sparseAddressSpaceSize: {}", limits.sparseAddressSpaceSize);
    spdlog::debug("maxBoundDescriptorSets: {}", limits.maxBoundDescriptorSets);
    spdlog::debug("maxPerStageDescriptorSamplers: {}", limits.maxPerStageDescriptorSamplers);
    spdlog::debug("maxPerStageDescriptorUniformBuffers: {}", limits.maxPerStageDescriptorUniformBuffers);
    spdlog::debug("maxPerStageDescriptorStorageBuffers: {}", limits.maxPerStageDescriptorStorageBuffers);
    spdlog::debug("maxPerStageDescriptorSampledImages: {}", limits.maxPerStageDescriptorSampledImages);
    spdlog::debug("maxPerStageDescriptorStorageImages: {}", limits.maxPerStageDescriptorStorageImages);
    spdlog::debug("maxPerStageDescriptorInputAttachments: {}", limits.maxPerStageDescriptorInputAttachments);
    spdlog::debug("maxPerStageResources: {}", limits.maxPerStageResources);
    spdlog::debug("maxDescriptorSetSamplers: {}", limits.maxDescriptorSetSamplers);
    spdlog::debug("maxDescriptorSetUniformBuffers: {}", limits.maxDescriptorSetUniformBuffers);
    spdlog::debug("maxDescriptorSetUniformBuffersDynamic: {}", limits.maxDescriptorSetUniformBuffersDynamic);
    spdlog::debug("maxDescriptorSetStorageBuffers: {}", limits.maxDescriptorSetStorageBuffers);
    spdlog::debug("maxDescriptorSetStorageBuffersDynamic: {}", limits.maxDescriptorSetStorageBuffersDynamic);
    spdlog::debug("maxDescriptorSetSampledImages: {}", limits.maxDescriptorSetSampledImages);
    spdlog::debug("maxDescriptorSetStorageImages: {}", limits.maxDescriptorSetStorageImages);
    spdlog::debug("maxDescriptorSetInputAttachments: {}", limits.maxDescriptorSetInputAttachments);
    spdlog::debug("maxVertexInputAttributes: {}", limits.maxVertexInputAttributes);
    spdlog::debug("maxVertexInputBindings: {}", limits.maxVertexInputBindings);
    spdlog::debug("maxVertexInputAttributeOffset: {}", limits.maxVertexInputAttributeOffset);
    spdlog::debug("maxVertexInputBindingStride: {}", limits.maxVertexInputBindingStride);
    spdlog::debug("maxVertexOutputComponents: {}", limits.maxVertexOutputComponents);
    spdlog::debug("maxTessellationGenerationLevel: {}", limits.maxTessellationGenerationLevel);
    spdlog::debug("maxTessellationPatchSize: {}", limits.maxTessellationPatchSize);
    spdlog::debug("maxTessellationControlPerVertexInputComponents: {}",
                  limits.maxTessellationControlPerVertexInputComponents);
    spdlog::debug("maxTessellationControlPerVertexOutputComponents: {}",
                  limits.maxTessellationControlPerVertexOutputComponents);
    spdlog::debug("maxTessellationControlPerPatchOutputComponents: {}",
                  limits.maxTessellationControlPerPatchOutputComponents);
    spdlog::debug("maxTessellationControlTotalOutputComponents: {}",
                  limits.maxTessellationControlTotalOutputComponents);
    spdlog::debug("maxTessellationEvaluationInputComponents: {}", limits.maxTessellationEvaluationInputComponents);
    spdlog::debug("maxTessellationEvaluationOutputComponents: {}", limits.maxTessellationEvaluationOutputComponents);
    spdlog::debug("maxGeometryShaderInvocations: {}", limits.maxGeometryShaderInvocations);
    spdlog::debug("maxGeometryInputComponents: {}", limits.maxGeometryInputComponents);
    spdlog::debug("maxGeometryOutputComponents: {}", limits.maxGeometryOutputComponents);
    spdlog::debug("maxGeometryOutputVertices: {}", limits.maxGeometryOutputVertices);
    spdlog::debug("maxGeometryTotalOutputComponents: {}", limits.maxGeometryTotalOutputComponents);
    spdlog::debug("maxFragmentInputComponents: {}", limits.maxFragmentInputComponents);
    spdlog::debug("maxFragmentOutputAttachments: {}", limits.maxFragmentOutputAttachments);
    spdlog::debug("maxFragmentDualSrcAttachments: {}", limits.maxFragmentDualSrcAttachments);
    spdlog::debug("maxFragmentCombinedOutputResources: {}", limits.maxFragmentCombinedOutputResources);
    spdlog::debug("maxComputeSharedMemorySize: {}", limits.maxComputeSharedMemorySize);
    spdlog::debug("maxComputeWorkGroupCount[0]: {}", limits.maxComputeWorkGroupCount[0]);
    spdlog::debug("maxComputeWorkGroupCount[1]: {}", limits.maxComputeWorkGroupCount[1]);
    spdlog::debug("maxComputeWorkGroupCount[2]: {}", limits.maxComputeWorkGroupCount[2]);
    spdlog::debug("maxComputeWorkGroupInvocations: {}", limits.maxComputeWorkGroupInvocations);
    spdlog::debug("maxComputeWorkGroupSize[0]: {}", limits.maxComputeWorkGroupSize[0]);
    spdlog::debug("maxComputeWorkGroupSize[1]: {}", limits.maxComputeWorkGroupSize[1]);
    spdlog::debug("maxComputeWorkGroupSize[2]: {}", limits.maxComputeWorkGroupSize[2]);
    spdlog::debug("subPixelPrecisionBits: {}", limits.subPixelPrecisionBits);
    spdlog::debug("subTexelPrecisionBits: {}", limits.subTexelPrecisionBits);
    spdlog::debug("mipmapPrecisionBits: {}", limits.mipmapPrecisionBits);
    spdlog::debug("maxDrawIndexedIndexValue: {}", limits.maxDrawIndexedIndexValue);
    spdlog::debug("maxDrawIndirectCount: {}", limits.maxDrawIndirectCount);
    spdlog::debug("maxSamplerLodBias: {}", limits.maxSamplerLodBias);
    spdlog::debug("maxSamplerAnisotropy: {}", limits.maxSamplerAnisotropy);
    spdlog::debug("maxViewports: {}", limits.maxViewports);
    spdlog::debug("maxViewportDimensions[0]: {}", limits.maxViewportDimensions[0]);
    spdlog::debug("maxViewportDimensions[1]: {}", limits.maxViewportDimensions[1]);
    spdlog::debug("viewportBoundsRange[0]: {}", limits.viewportBoundsRange[0]);
    spdlog::debug("viewportBoundsRange[1]: {}", limits.viewportBoundsRange[1]);
    spdlog::debug("viewportSubPixelBits: {}", limits.viewportSubPixelBits);
    spdlog::debug("minMemoryMapAlignment: {}", limits.minMemoryMapAlignment);
    spdlog::debug("minTexelBufferOffsetAlignment: {}", limits.minTexelBufferOffsetAlignment);
    spdlog::debug("minUniformBufferOffsetAlignment: {}", limits.minUniformBufferOffsetAlignment);
    spdlog::debug("minStorageBufferOffsetAlignment: {}", limits.minStorageBufferOffsetAlignment);
    spdlog::debug("minTexelOffset: {}", limits.minTexelOffset);
    spdlog::debug("maxTexelOffset: {}", limits.maxTexelOffset);
    spdlog::debug("minTexelGatherOffset: {}", limits.minTexelGatherOffset);
    spdlog::debug("maxTexelGatherOffset: {}", limits.maxTexelGatherOffset);
    spdlog::debug("minInterpolationOffset: {}", limits.minInterpolationOffset);
    spdlog::debug("maxInterpolationOffset: {}", limits.maxInterpolationOffset);
    spdlog::debug("subPixelInterpolationOffsetBits: {}", limits.subPixelInterpolationOffsetBits);
    spdlog::debug("maxFramebufferWidth: {}", limits.maxFramebufferWidth);
    spdlog::debug("maxFramebufferHeight: {}", limits.maxFramebufferHeight);
    spdlog::debug("maxFramebufferLayers: {}", limits.maxFramebufferLayers);
    spdlog::debug("framebufferColorSampleCounts: {}", limits.framebufferColorSampleCounts);
    spdlog::debug("framebufferDepthSampleCounts: {}", limits.framebufferDepthSampleCounts);
    spdlog::debug("framebufferStencilSampleCounts: {}", limits.framebufferStencilSampleCounts);
    spdlog::debug("framebufferNoAttachmentsSampleCounts: {}", limits.framebufferNoAttachmentsSampleCounts);
    spdlog::debug("maxColorAttachments: {}", limits.maxColorAttachments);
    spdlog::debug("sampledImageColorSampleCounts: {}", limits.sampledImageColorSampleCounts);
    spdlog::debug("sampledImageIntegerSampleCounts: {}", limits.sampledImageIntegerSampleCounts);
    spdlog::debug("sampledImageDepthSampleCounts: {}", limits.sampledImageDepthSampleCounts);
    spdlog::debug("sampledImageStencilSampleCounts: {}", limits.sampledImageStencilSampleCounts);
    spdlog::debug("storageImageSampleCounts: {}", limits.storageImageSampleCounts);
    spdlog::debug("maxSampleMaskWords: {}", limits.maxSampleMaskWords);
    spdlog::debug("timestampComputeAndGraphics: {}", limits.timestampComputeAndGraphics);
    spdlog::debug("timestampPeriod: {}", limits.timestampPeriod);
    spdlog::debug("maxClipDistances: {}", limits.maxClipDistances);
    spdlog::debug("maxCullDistances: {}", limits.maxCullDistances);
    spdlog::debug("maxCombinedClipAndCullDistances: {}", limits.maxCombinedClipAndCullDistances);
    spdlog::debug("discreteQueuePriorities: {}", limits.discreteQueuePriorities);
    spdlog::debug("pointSizeRange[0]: {}", limits.pointSizeRange[0]);
    spdlog::debug("pointSizeRange[1]: {}", limits.pointSizeRange[1]);
    spdlog::debug("lineWidthRange[0]: {}", limits.lineWidthRange[0]);
    spdlog::debug("lineWidthRange[1]: {}", limits.lineWidthRange[1]);
    spdlog::debug("pointSizeGranularity: {}", limits.pointSizeGranularity);
    spdlog::debug("lineWidthGranularity: {}", limits.lineWidthGranularity);
    spdlog::debug("strictLines: {}", limits.strictLines);
    spdlog::debug("standardSampleLocations: {}", limits.standardSampleLocations);
    spdlog::debug("optimalBufferCopyOffsetAlignment: {}", limits.optimalBufferCopyOffsetAlignment);
    spdlog::debug("optimalBufferCopyRowPitchAlignment: {}", limits.optimalBufferCopyRowPitchAlignment);
    spdlog::debug("nonCoherentAtomSize: {}", limits.nonCoherentAtomSize);
}

void print_all_physical_devices(const VkInstance instance, const VkSurfaceKHR surface) {
    assert(instance);
    assert(surface);

    std::uint32_t gpu_count{0};

    // Query how many graphics cards are available.
    if (const auto result = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumeratePhysicalDevices returned {}!", vk_tools::as_string(result));
        return;
    }

    spdlog::debug("Number of available gpus: {}", gpu_count);

    if (gpu_count == 0) {
        return;
    }

    std::vector<VkPhysicalDevice> available_gpus;
    available_gpus.reserve(gpu_count);

    // Store all available graphics cards.
    if (const auto result = vkEnumeratePhysicalDevices(instance, &gpu_count, available_gpus.data());
        result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumeratePhysicalDevices returned {}!", vk_tools::as_string(result));
        return;
    }

    for (auto *gpu : available_gpus) {
        print_device_extensions(gpu);
        print_physical_device_info(gpu);
        print_physical_device_queue_families(gpu);
        print_surface_capabilities(gpu, surface);
        print_supported_surface_formats(gpu, surface);
        print_presentation_modes(gpu, surface);
        print_physical_device_memory_properties(gpu);
        print_physical_device_features(gpu);
        print_physical_device_sparse_properties(gpu);
        print_physical_device_limits(gpu);
    }
}

} // namespace inexor::vulkan_renderer::vk_tools
