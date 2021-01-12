#include "inexor/vulkan-renderer/gpu_info/gpu_info.hpp"

#include "inexor/vulkan-renderer/exceptions/vk_exception.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <cstdint>

namespace inexor::vulkan_renderer::gpu_info {

std::string get_present_mode_name(const VkPresentModeKHR present_mode) {
    switch (present_mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return "VK_PRESENT_MODE_IMMEDIATE_KHR";
    case VK_PRESENT_MODE_MAILBOX_KHR:
        return "VK_PRESENT_MODE_MAILBOX_KHR";
    case VK_PRESENT_MODE_FIFO_KHR:
        return "VK_PRESENT_MODE_FIFO_KHR";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
        return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
    default:
        break;
    }

    // If no name can be found, convert the present mode's value to std::string and return it.
    return std::to_string(present_mode);
}

std::string get_graphics_card_type(const VkPhysicalDeviceType gpu_type) {
    switch (gpu_type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "VK_PHYSICAL_DEVICE_TYPE_CPU";
    default:
        break;
    }

    // If no name can be found, convert the physical device type value to a std::string and return it.
    return std::to_string(gpu_type);
}

std::string get_vkformat_name(const VkFormat format) {
    switch (format) {
    case VK_FORMAT_UNDEFINED:
        return "VK_FORMAT_UNDEFINED";
    case VK_FORMAT_R4G4_UNORM_PACK8:
        return "VK_FORMAT_R4G4_UNORM_PACK8";
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        return "VK_FORMAT_R5G6B5_UNORM_PACK16";
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
        return "VK_FORMAT_B5G6R5_UNORM_PACK16";
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
    case VK_FORMAT_R8_UNORM:
        return "VK_FORMAT_R8_UNORM";
    case VK_FORMAT_R8_SNORM:
        return "VK_FORMAT_R8_SNORM";
    case VK_FORMAT_R8_USCALED:
        return "VK_FORMAT_R8_USCALED";
    case VK_FORMAT_R8_SSCALED:
        return "VK_FORMAT_R8_SSCALED";
    case VK_FORMAT_R8_UINT:
        return "VK_FORMAT_R8_UINT";
    case VK_FORMAT_R8_SINT:
        return "VK_FORMAT_R8_SINT";
    case VK_FORMAT_R8_SRGB:
        return "VK_FORMAT_R8_SRGB";
    case VK_FORMAT_R8G8_UNORM:
        return "VK_FORMAT_R8G8_UNORM";
    case VK_FORMAT_R8G8_SNORM:
        return "VK_FORMAT_R8G8_SNORM";
    case VK_FORMAT_R8G8_USCALED:
        return "VK_FORMAT_R8G8_USCALED";
    case VK_FORMAT_R8G8_SSCALED:
        return "VK_FORMAT_R8G8_SSCALED";
    case VK_FORMAT_R8G8_UINT:
        return "VK_FORMAT_R8G8_UINT";
    case VK_FORMAT_R8G8_SINT:
        return "VK_FORMAT_R8G8_SINT";
    case VK_FORMAT_R8G8_SRGB:
        return "VK_FORMAT_R8G8_SRGB";
    case VK_FORMAT_R8G8B8_UNORM:
        return "VK_FORMAT_R8G8B8_UNORM";
    case VK_FORMAT_R8G8B8_SNORM:
        return "VK_FORMAT_R8G8B8_SNORM";
    case VK_FORMAT_R8G8B8_USCALED:
        return "VK_FORMAT_R8G8B8_USCALED";
    case VK_FORMAT_R8G8B8_SSCALED:
        return "VK_FORMAT_R8G8B8_SSCALED";
    case VK_FORMAT_R8G8B8_UINT:
        return "VK_FORMAT_R8G8B8_UINT";
    case VK_FORMAT_R8G8B8_SINT:
        return "VK_FORMAT_R8G8B8_SINT";
    case VK_FORMAT_R8G8B8_SRGB:
        return "VK_FORMAT_R8G8B8_SRGB";
    case VK_FORMAT_B8G8R8_UNORM:
        return "VK_FORMAT_B8G8R8_UNORM";
    case VK_FORMAT_B8G8R8_SNORM:
        return "VK_FORMAT_B8G8R8_SNORM";
    case VK_FORMAT_B8G8R8_USCALED:
        return "VK_FORMAT_B8G8R8_USCALED";
    case VK_FORMAT_B8G8R8_SSCALED:
        return "VK_FORMAT_B8G8R8_SSCALED";
    case VK_FORMAT_B8G8R8_UINT:
        return "VK_FORMAT_B8G8R8_UINT";
    case VK_FORMAT_B8G8R8_SINT:
        return "VK_FORMAT_B8G8R8_SINT";
    case VK_FORMAT_B8G8R8_SRGB:
        return "VK_FORMAT_B8G8R8_SRGB";
    case VK_FORMAT_R8G8B8A8_UNORM:
        return "VK_FORMAT_R8G8B8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_SNORM:
        return "VK_FORMAT_R8G8B8A8_SNORM";
    case VK_FORMAT_R8G8B8A8_USCALED:
        return "VK_FORMAT_R8G8B8A8_USCALED";
    case VK_FORMAT_R8G8B8A8_SSCALED:
        return "VK_FORMAT_R8G8B8A8_SSCALED";
    case VK_FORMAT_R8G8B8A8_UINT:
        return "VK_FORMAT_R8G8B8A8_UINT";
    case VK_FORMAT_R8G8B8A8_SINT:
        return "VK_FORMAT_R8G8B8A8_SINT";
    case VK_FORMAT_R8G8B8A8_SRGB:
        return "VK_FORMAT_R8G8B8A8_SRGB";
    case VK_FORMAT_B8G8R8A8_UNORM:
        return "VK_FORMAT_B8G8R8A8_UNORM";
    case VK_FORMAT_B8G8R8A8_SNORM:
        return "VK_FORMAT_B8G8R8A8_SNORM";
    case VK_FORMAT_B8G8R8A8_USCALED:
        return "VK_FORMAT_B8G8R8A8_USCALED";
    case VK_FORMAT_B8G8R8A8_SSCALED:
        return "VK_FORMAT_B8G8R8A8_SSCALED";
    case VK_FORMAT_B8G8R8A8_UINT:
        return "VK_FORMAT_B8G8R8A8_UINT";
    case VK_FORMAT_B8G8R8A8_SINT:
        return "VK_FORMAT_B8G8R8A8_SINT";
    case VK_FORMAT_B8G8R8A8_SRGB:
        return "VK_FORMAT_B8G8R8A8_SRGB";
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
    case VK_FORMAT_R16_UNORM:
        return "VK_FORMAT_R16_UNORM";
    case VK_FORMAT_R16_SNORM:
        return "VK_FORMAT_R16_SNORM";
    case VK_FORMAT_R16_USCALED:
        return "VK_FORMAT_R16_USCALED";
    case VK_FORMAT_R16_SSCALED:
        return "VK_FORMAT_R16_SSCALED";
    case VK_FORMAT_R16_UINT:
        return "VK_FORMAT_R16_UINT";
    case VK_FORMAT_R16_SINT:
        return "VK_FORMAT_R16_SINT";
    case VK_FORMAT_R16_SFLOAT:
        return "VK_FORMAT_R16_SFLOAT";
    case VK_FORMAT_R16G16_UNORM:
        return "VK_FORMAT_R16G16_UNORM";
    case VK_FORMAT_R16G16_SNORM:
        return "VK_FORMAT_R16G16_SNORM";
    case VK_FORMAT_R16G16_USCALED:
        return "VK_FORMAT_R16G16_USCALED";
    case VK_FORMAT_R16G16_SSCALED:
        return "VK_FORMAT_R16G16_SSCALED";
    case VK_FORMAT_R16G16_UINT:
        return "VK_FORMAT_R16G16_UINT";
    case VK_FORMAT_R16G16_SINT:
        return "VK_FORMAT_R16G16_SINT";
    case VK_FORMAT_R16G16_SFLOAT:
        return "VK_FORMAT_R16G16_SFLOAT";
    case VK_FORMAT_R16G16B16_UNORM:
        return "VK_FORMAT_R16G16B16_UNORM";
    case VK_FORMAT_R16G16B16_SNORM:
        return "VK_FORMAT_R16G16B16_SNORM";
    case VK_FORMAT_R16G16B16_USCALED:
        return "VK_FORMAT_R16G16B16_USCALED";
    case VK_FORMAT_R16G16B16_SSCALED:
        return "VK_FORMAT_R16G16B16_SSCALED";
    case VK_FORMAT_R16G16B16_UINT:
        return "VK_FORMAT_R16G16B16_UINT";
    case VK_FORMAT_R16G16B16_SINT:
        return "VK_FORMAT_R16G16B16_SINT";
    case VK_FORMAT_R16G16B16_SFLOAT:
        return "VK_FORMAT_R16G16B16_SFLOAT";
    case VK_FORMAT_R16G16B16A16_UNORM:
        return "VK_FORMAT_R16G16B16A16_UNORM";
    case VK_FORMAT_R16G16B16A16_SNORM:
        return "VK_FORMAT_R16G16B16A16_SNORM";
    case VK_FORMAT_R16G16B16A16_USCALED:
        return "VK_FORMAT_R16G16B16A16_USCALED";
    case VK_FORMAT_R16G16B16A16_SSCALED:
        return "VK_FORMAT_R16G16B16A16_SSCALED";
    case VK_FORMAT_R16G16B16A16_UINT:
        return "VK_FORMAT_R16G16B16A16_UINT";
    case VK_FORMAT_R16G16B16A16_SINT:
        return "VK_FORMAT_R16G16B16A16_SINT";
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return "VK_FORMAT_R16G16B16A16_SFLOAT";
    case VK_FORMAT_R32_UINT:
        return "VK_FORMAT_R32_UINT";
    case VK_FORMAT_R32_SINT:
        return "VK_FORMAT_R32_SINT";
    case VK_FORMAT_R32_SFLOAT:
        return "VK_FORMAT_R32_SFLOAT";
    case VK_FORMAT_R32G32_UINT:
        return "VK_FORMAT_R32G32_UINT";
    case VK_FORMAT_R32G32_SINT:
        return "VK_FORMAT_R32G32_SINT";
    case VK_FORMAT_R32G32_SFLOAT:
        return "VK_FORMAT_R32G32_SFLOAT";
    case VK_FORMAT_R32G32B32_UINT:
        return "VK_FORMAT_R32G32B32_UINT";
    case VK_FORMAT_R32G32B32_SINT:
        return "VK_FORMAT_R32G32B32_SINT";
    case VK_FORMAT_R32G32B32_SFLOAT:
        return "VK_FORMAT_R32G32B32_SFLOAT";
    case VK_FORMAT_R32G32B32A32_UINT:
        return "VK_FORMAT_R32G32B32A32_UINT";
    case VK_FORMAT_R32G32B32A32_SINT:
        return "VK_FORMAT_R32G32B32A32_SINT";
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return "VK_FORMAT_R32G32B32A32_SFLOAT";
    case VK_FORMAT_R64_UINT:
        return "VK_FORMAT_R64_UINT";
    case VK_FORMAT_R64_SINT:
        return "VK_FORMAT_R64_SINT";
    case VK_FORMAT_R64_SFLOAT:
        return "VK_FORMAT_R64_SFLOAT";
    case VK_FORMAT_R64G64_UINT:
        return "VK_FORMAT_R64G64_UINT";
    case VK_FORMAT_R64G64_SINT:
        return "VK_FORMAT_R64G64_SINT";
    case VK_FORMAT_R64G64_SFLOAT:
        return "VK_FORMAT_R64G64_SFLOAT";
    case VK_FORMAT_R64G64B64_UINT:
        return "VK_FORMAT_R64G64B64_UINT";
    case VK_FORMAT_R64G64B64_SINT:
        return "VK_FORMAT_R64G64B64_SINT";
    case VK_FORMAT_R64G64B64_SFLOAT:
        return "VK_FORMAT_R64G64B64_SFLOAT";
    case VK_FORMAT_R64G64B64A64_UINT:
        return "VK_FORMAT_R64G64B64A64_UINT";
    case VK_FORMAT_R64G64B64A64_SINT:
        return "VK_FORMAT_R64G64B64A64_SINT";
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return "VK_FORMAT_R64G64B64A64_SFLOAT";
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
    case VK_FORMAT_D16_UNORM:
        return "VK_FORMAT_D16_UNORM";
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return "VK_FORMAT_X8_D24_UNORM_PACK32";
    case VK_FORMAT_D32_SFLOAT:
        return "VK_FORMAT_D32_SFLOAT";
    case VK_FORMAT_S8_UINT:
        return "VK_FORMAT_S8_UINT";
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return "VK_FORMAT_D16_UNORM_S8_UINT";
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return "VK_FORMAT_D24_UNORM_S8_UINT";
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return "VK_FORMAT_D32_SFLOAT_S8_UINT";
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
    case VK_FORMAT_BC2_UNORM_BLOCK:
        return "VK_FORMAT_BC2_UNORM_BLOCK";
    case VK_FORMAT_BC2_SRGB_BLOCK:
        return "VK_FORMAT_BC2_SRGB_BLOCK";
    case VK_FORMAT_BC3_UNORM_BLOCK:
        return "VK_FORMAT_BC3_UNORM_BLOCK";
    case VK_FORMAT_BC3_SRGB_BLOCK:
        return "VK_FORMAT_BC3_SRGB_BLOCK";
    case VK_FORMAT_BC4_UNORM_BLOCK:
        return "VK_FORMAT_BC4_UNORM_BLOCK";
    case VK_FORMAT_BC4_SNORM_BLOCK:
        return "VK_FORMAT_BC4_SNORM_BLOCK";
    case VK_FORMAT_BC5_UNORM_BLOCK:
        return "VK_FORMAT_BC5_UNORM_BLOCK";
    case VK_FORMAT_BC5_SNORM_BLOCK:
        return "VK_FORMAT_BC5_SNORM_BLOCK";
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
    case VK_FORMAT_BC7_UNORM_BLOCK:
        return "VK_FORMAT_BC7_UNORM_BLOCK";
    case VK_FORMAT_BC7_SRGB_BLOCK:
        return "VK_FORMAT_BC7_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
    case VK_FORMAT_G8B8G8R8_422_UNORM:
        return "VK_FORMAT_G8B8G8R8_422_UNORM";
    case VK_FORMAT_B8G8R8G8_422_UNORM:
        return "VK_FORMAT_B8G8R8G8_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
    case VK_FORMAT_R10X6_UNORM_PACK16:
        return "VK_FORMAT_R10X6_UNORM_PACK16";
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_R12X4_UNORM_PACK16:
        return "VK_FORMAT_R12X4_UNORM_PACK16";
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_G16B16G16R16_422_UNORM:
        return "VK_FORMAT_G16B16G16R16_422_UNORM";
    case VK_FORMAT_B16G16R16G16_422_UNORM:
        return "VK_FORMAT_B16G16R16G16_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT";
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
        return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT";
    default:
        break;
    }

    // If not name can be found, convert the format value to std::string and return it.
    return std::to_string(format);
}

void print_driver_vulkan_version() {
    std::uint32_t api_version = 0;

    if (const auto result = vkEnumerateInstanceVersion(&api_version); result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateInstanceVersion failed!", result);
    }

    spdlog::debug("Supported Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(api_version),
                  VK_VERSION_MINOR(api_version), VK_VERSION_PATCH(api_version));
}

void print_physical_device_queue_families(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t queue_family_count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &queue_family_count, nullptr);

    spdlog::debug("Number of queue families: {}", queue_family_count);

    if (queue_family_count == 0) {
        spdlog::error("Error: Could not find any queue families!");
    }

    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &queue_family_count, queue_family_properties.data());

    for (std::size_t i = 0; i < queue_family_count; i++) {
        spdlog::debug("Queue family: {}", i);
        spdlog::debug("Queue count: {}", queue_family_properties[i].queueCount);
        spdlog::debug("Timestamp valid bits: {}", queue_family_properties[i].timestampValidBits);

        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            spdlog::debug("VK_QUEUE_GRAPHICS_BIT");
        }
        if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            spdlog::debug("VK_QUEUE_COMPUTE_BIT");
        }
        if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            spdlog::debug("VK_QUEUE_TRANSFER_BIT");
        }
        if (queue_family_properties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            spdlog::debug("VK_QUEUE_SPARSE_BINDING_BIT");
        }
        if (queue_family_properties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
            spdlog::debug("VK_QUEUE_PROTECTED_BIT");
        }

        spdlog::debug("Min image timestamp granularity: width {}, heigth {}, depth {}",
                      queue_family_properties[i].minImageTransferGranularity.width,
                      queue_family_properties[i].minImageTransferGranularity.height,
                      queue_family_properties[i].minImageTransferGranularity.depth);
    }
}

void print_instance_layers() {
    std::uint32_t instance_layer_count = 0;

    // Query how many instance layers are available.
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr); result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
    }

    spdlog::debug("Number of instance layers: {}", instance_layer_count);

    if (instance_layer_count == 0) {
        throw std::runtime_error("Error: Could not find any instance layers!");
    }

    std::vector<VkLayerProperties> instance_layers(instance_layer_count);

    // Store all available instance layers.
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
    }

    for (const auto &instance_layer : instance_layers) {
        spdlog::debug("Name: {}", instance_layer.layerName);
        spdlog::debug("Spec Version: {}", VK_VERSION_MAJOR(instance_layer.specVersion),
                      VK_VERSION_MINOR(instance_layer.specVersion), VK_VERSION_PATCH(instance_layer.specVersion));
        spdlog::debug("Impl Version: {}", instance_layer.implementationVersion);
        spdlog::debug("Description: {}", instance_layer.description);
    }
}

void print_instance_extensions() {
    std::uint32_t instance_extension_count = 0;

    // Query how many instance extensions are available.
    if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
    }

    spdlog::debug("Number of instance extensions: {} ", instance_extension_count);

    if (instance_extension_count == 0) {
        throw std::runtime_error("Error: Could not find any instance extensions!");
    }

    std::vector<VkExtensionProperties> extensions(instance_extension_count);

    // Store all available instance extensions.
    if (const auto result =
            vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, extensions.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
    }

    for (const auto &extension : extensions) {
        spdlog::debug("Spec version: {}.{}.{}\t Name: {}", VK_VERSION_MAJOR(extension.specVersion),
                      VK_VERSION_MINOR(extension.specVersion), VK_VERSION_PATCH(extension.specVersion),
                      extension.extensionName);
    }
}

void print_device_layers(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t device_layer_count = 0;

    // Query how many device layers are available.
    if (const auto result = vkEnumerateDeviceLayerProperties(graphics_card, &device_layer_count, nullptr);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateDeviceLayerProperties failed!", result);
    }

    spdlog::debug("Number of device layers: {}", device_layer_count);

    if (device_layer_count == 0) {
        throw std::runtime_error("Error: Could not find any device layers!");
    }

    std::vector<VkLayerProperties> device_layers(device_layer_count);

    // Store all available device layers.
    if (const auto result = vkEnumerateDeviceLayerProperties(graphics_card, &device_layer_count, device_layers.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateDeviceLayerProperties failed!", result);
    }

    for (const auto &device_layer : device_layers) {
        spdlog::debug("Name: {}", device_layer.layerName);
        spdlog::debug("Spec Version: {}.{}.{}", VK_VERSION_MAJOR(device_layer.specVersion),
                      VK_VERSION_MINOR(device_layer.specVersion), VK_VERSION_PATCH(device_layer.specVersion));
        spdlog::debug("Impl Version: {}", device_layer.implementationVersion);
        spdlog::debug("Description: {}", device_layer.description);
    }
}

void print_device_extensions(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t device_extension_count = 0;

    // First check how many device extensions are available.
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    spdlog::debug("Number of device extensions: {}", device_extension_count);

    if (device_extension_count == 0) {
        throw std::runtime_error("Error: Could not find any device extensions!");
    }

    std::vector<VkExtensionProperties> device_extensions(device_extension_count);

    // Store all available device extensions.
    if (const auto result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count,
                                                                 device_extensions.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    for (const auto &device_extension : device_extensions) {
        spdlog::debug("Spec version: {}.{}.{}\t Name: {}", VK_VERSION_MAJOR(device_extension.specVersion),
                      VK_VERSION_MINOR(device_extension.specVersion), VK_VERSION_PATCH(device_extension.specVersion),
                      device_extension.extensionName);
    }
}

void print_surface_capabilities(const VkPhysicalDevice graphics_card, const VkSurfaceKHR vulkan_surface) {
    assert(graphics_card);
    assert(vulkan_surface);

    spdlog::debug("Printing surface capabilities.");

    VkSurfaceCapabilitiesKHR surface_capabilities;

    if (const auto result =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, vulkan_surface, &surface_capabilities);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
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

void print_supported_surface_formats(const VkPhysicalDevice graphics_card, const VkSurfaceKHR vulkan_surface) {
    assert(graphics_card);
    assert(vulkan_surface);

    std::uint32_t format_count = 0;

    // Query how many formats are supported.
    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &format_count, nullptr);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }

    spdlog::debug("Supported surface formats: {}", format_count);

    if (format_count == 0) {
        spdlog::critical("Error: Could not find any supported formats!");
    }

    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);

    if (const auto result =
            vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &format_count, surface_formats.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }

    for (const auto format : surface_formats) {
        spdlog::debug("Surface format: {}", get_vkformat_name(format.format));
    }
}

void print_presentation_modes(const VkPhysicalDevice graphics_card, const VkSurfaceKHR vulkan_surface) {
    assert(graphics_card);
    assert(vulkan_surface);

    std::uint32_t present_mode_count = 0;

    // Query how many presentation modes are available.
    if (const auto result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &present_mode_count, nullptr);
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }

    spdlog::debug("Available present modes: ", present_mode_count);

    if (present_mode_count == 0) {
        throw std::runtime_error("Error: Could not find any presentation modes!");
    }

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);

    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface,
                                                                      &present_mode_count, present_modes.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }

    for (const auto mode : present_modes) {
        spdlog::debug("Present mode: {}", get_present_mode_name(mode));
    }
}

void print_physical_device_info(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(graphics_card, &gpu_properties);

    spdlog::debug("Graphics card: {}", gpu_properties.deviceName);

    spdlog::debug("Vulkan API supported version: {}.{}.{}", VK_VERSION_MAJOR(gpu_properties.apiVersion),
                  VK_VERSION_MINOR(gpu_properties.apiVersion), VK_VERSION_PATCH(gpu_properties.apiVersion));

    // The driver version format is nost standardised!
    spdlog::debug("Vulkan API supported version: {}.{}.{}", VK_VERSION_MAJOR(gpu_properties.driverVersion),
                  VK_VERSION_MINOR(gpu_properties.driverVersion), VK_VERSION_PATCH(gpu_properties.driverVersion));
    spdlog::debug("Vendor ID: {}", gpu_properties.vendorID);
    spdlog::debug("Device ID: {}", gpu_properties.deviceID);
    spdlog::debug("Device type: {}", get_graphics_card_type(gpu_properties.deviceType));
}

void print_physical_device_memory_properties(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    spdlog::debug("Graphics card's memory properties:");

    VkPhysicalDeviceMemoryProperties gpu_memory_properties;

    vkGetPhysicalDeviceMemoryProperties(graphics_card, &gpu_memory_properties);

    spdlog::debug("Number of memory types: {}", gpu_memory_properties.memoryTypeCount);
    spdlog::debug("Number of heap types: {}", gpu_memory_properties.memoryHeapCount);

    for (std::size_t i = 0; i < gpu_memory_properties.memoryTypeCount; i++) {
        spdlog::debug("[{}] Heap index: {}", i, gpu_memory_properties.memoryTypes[i].heapIndex);

        const auto &property_flag = gpu_memory_properties.memoryTypes[i].propertyFlags;

        if (property_flag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_HOST_CACHED_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            spdlog::debug("VK_MEMORY_PROPERTY_PROTECTED_BIT");
        }
        if (property_flag & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
        }
        if (property_flag & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) {
            spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
        }
    }

    for (std::size_t i = 0; i < gpu_memory_properties.memoryHeapCount; i++) {
        spdlog::debug("Heap [{}], memory size: {}", i, gpu_memory_properties.memoryHeaps[i].size / (1000 * 1000));

        const auto &property_flag = gpu_memory_properties.memoryHeaps[i].flags;

        if (property_flag & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            spdlog::debug("VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ");
        }
        if (property_flag & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
            spdlog::debug("VK_MEMORY_HEAP_MULTI_INSTANCE_BIT");
        }
    }
}

void print_physical_device_features(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    VkPhysicalDeviceFeatures gpu_features;

    vkGetPhysicalDeviceFeatures(graphics_card, &gpu_features);

    spdlog::debug("Graphics card's features:");

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

void print_physical_device_sparse_properties(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(graphics_card, &gpu_properties);

    spdlog::debug("Graphics card's sparse properties:");

    spdlog::debug("residencyStandard2DBlockShape: {}", gpu_properties.sparseProperties.residencyStandard2DBlockShape);
    spdlog::debug("residencyStandard2DMultisampleBlockShape: {}",
                  gpu_properties.sparseProperties.residencyStandard2DMultisampleBlockShape);
    spdlog::debug("residencyStandard3DBlockShape: {}", gpu_properties.sparseProperties.residencyStandard3DBlockShape);
    spdlog::debug("residencyAlignedMipSize: {}", gpu_properties.sparseProperties.residencyAlignedMipSize);
    spdlog::debug("residencyNonResidentStrict: {}", gpu_properties.sparseProperties.residencyNonResidentStrict);
}

void print_physical_device_limits(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    VkPhysicalDeviceProperties gpu_properties;

    vkGetPhysicalDeviceProperties(graphics_card, &gpu_properties);

    spdlog::debug("Graphics card's limits:");

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

void print_all_physical_devices(const VkInstance vulkan_instance, const VkSurfaceKHR vulkan_surface) {
    assert(vulkan_surface);
    assert(vulkan_instance);

    std::uint32_t gpu_count = 0;

    // Query how many graphics cards are available.
    if (const auto result = vkEnumeratePhysicalDevices(vulkan_instance, &gpu_count, nullptr); result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }

    if (gpu_count == 0) {
        throw std::runtime_error("Error: Could not find any GPU's!");
    }

    spdlog::debug("Number of available graphics cards: {}", gpu_count);

    std::vector<VkPhysicalDevice> available_graphics_cards(gpu_count);

    // Store all available graphics cards.
    if (const auto result = vkEnumeratePhysicalDevices(vulkan_instance, &gpu_count, available_graphics_cards.data());
        result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }

    for (auto *graphics_card : available_graphics_cards) {
        print_device_layers(graphics_card);
        print_device_extensions(graphics_card);
        print_physical_device_info(graphics_card);
        print_physical_device_queue_families(graphics_card);
        print_surface_capabilities(graphics_card, vulkan_surface);
        print_supported_surface_formats(graphics_card, vulkan_surface);
        print_presentation_modes(graphics_card, vulkan_surface);
        print_physical_device_memory_properties(graphics_card);
        print_physical_device_features(graphics_card);
        print_physical_device_sparse_properties(graphics_card);
        print_physical_device_limits(graphics_card);
    }
}

} // namespace inexor::vulkan_renderer::gpu_info
