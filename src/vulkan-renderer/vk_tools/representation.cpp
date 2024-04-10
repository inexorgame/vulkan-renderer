#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

#include <array>
#include <cassert>

namespace inexor::vulkan_renderer::vk_tools {

// Please keep the functions in here in alphabetical order (sort by function names, then by parameter types)

/// Convert a VkCompositeAlphaFlagBitsKHR into the corresponding std::string_view
/// @param flag The composite alpha flag bit
/// @return A std::string_view which contains the composite alpha flag bit
template <>
std::string_view as_string(const VkCompositeAlphaFlagBitsKHR flag) {
    switch (flag) {
    case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR:
        return "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR";
    case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR:
        return "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR";
    case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR:
        return "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR";
    case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR:
        return "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR";
    default:
        return "Unknown VkCompositeAlphaFlagBitsKHR";
    }
}

/// Convert a VkFormat value into the corresponding std::string_view
/// @param format The VkFormat to convert
/// @return A std::string_view which contains the format
template <>
std::string_view as_string(const VkFormat format) {
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
        return "Unknown VkFormat";
    }
}

/// Convert a VkImageUsageFlagBits value into the corresponding std::string_view
/// @param img_usage_flag_bits The image usage flag bit
/// @return A std::string_view which contains the image usage flag bit
template <>
std::string_view as_string(const VkImageUsageFlagBits img_usage_flag_bits) {
    switch (img_usage_flag_bits) {
    case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
        return "VK_IMAGE_USAGE_TRANSFER_SRC_BIT";
    case VK_IMAGE_USAGE_TRANSFER_DST_BIT:
        return "VK_IMAGE_USAGE_TRANSFER_DST_BIT";
    case VK_IMAGE_USAGE_SAMPLED_BIT:
        return "VK_IMAGE_USAGE_SAMPLED_BIT";
    case VK_IMAGE_USAGE_STORAGE_BIT:
        return "VK_IMAGE_USAGE_STORAGE_BIT";
    case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
        return "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:
        return "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT";
    case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT:
        return "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT";
    case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT:
        return "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ";
    default:
        return "Unknown VkImageUsageFlagBits";
    }
}

/// Convert a VkMemoryPropertyFlagBits value into the corresponding std::string_view
/// @param mem_prop_flag_bit The VkMemoryPropertyFlagBits to convert
/// @return A std::string_view which contains the memory property flag bit
template <>
std::string_view as_string(const VkMemoryPropertyFlagBits mem_prop_flag_bit) {
    switch (mem_prop_flag_bit) {
    case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
        return "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT";
    case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:
        return "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT";
    case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
        return "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT";
    case VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
        return "VK_MEMORY_PROPERTY_HOST_CACHED_BIT";
    case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:
        return "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT";
    case VK_MEMORY_PROPERTY_PROTECTED_BIT:
        return "VK_MEMORY_PROPERTY_PROTECTED_BIT";
    case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD:
        return "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD";
    case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD:
        return "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD";
    case VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM:
        return "VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM";
    default:
        return "Unknown VkMemoryPropertyFlagBits";
    }
}

/// Convert a VkMemoryHeapFlagBits value into the corresponding std::string_view
/// @param mem_heap_flag_bit The VkMemoryHeapFlagBits to convert
/// @return A std::string_view which contains the memory heap flag bit
template <>
std::string_view as_string(const VkMemoryHeapFlagBits mem_heap_flag_bit) {
    switch (mem_heap_flag_bit) {
    case VK_MEMORY_HEAP_DEVICE_LOCAL_BIT:
        return "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT";
    case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT: // same as VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR
        return "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT";
    case VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM:
        return "VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM";
    default:
        return "Unknown VkMemoryHeapFlagBits";
    }
}

/// Convert a VkPhysicalDeviceType value into the corresponding std::string_view
/// @param gpu_type The VkPhysicalDeviceType to convert
/// @return A std::string_view which contains the physical device type
template <>
std::string_view as_string(const VkPhysicalDeviceType gpu_type) {
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
        return "Unknown VkPhysicalDeviceType";
    }
}

/// Convert a VkPresentModeKHR value into the corresponding std::string_view
/// @param present_mode The VkPresentModeKHR to convert
/// @return A std::string_view which contains the presentation mode
template <>
std::string_view as_string(const VkPresentModeKHR present_mode) {
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
        return "Unknown VkPresentModeKHR";
    }
}

/// Convert a VkQueueFlagBits value into the corresponding std::string_view
/// @param queue_flag_bit The VkQueueFlagBits to convert
/// @return A std::string_view which contains the queue flag bit
template <>
std::string_view as_string(const VkQueueFlagBits queue_flag_bit) {
    switch (queue_flag_bit) {
    case VK_QUEUE_GRAPHICS_BIT:
        return "VK_QUEUE_GRAPHICS_BIT";
    case VK_QUEUE_COMPUTE_BIT:
        return "VK_QUEUE_COMPUTE_BIT";
    case VK_QUEUE_TRANSFER_BIT:
        return "VK_QUEUE_TRANSFER_BIT";
    case VK_QUEUE_SPARSE_BINDING_BIT:
        return "VK_QUEUE_SPARSE_BINDING_BIT";
    case VK_QUEUE_PROTECTED_BIT:
        return "VK_QUEUE_PROTECTED_BIT";
    case VK_QUEUE_FLAG_BITS_MAX_ENUM:
        return "VK_QUEUE_FLAG_BITS_MAX_ENUM";
    default:
        return "Unknown VkQueueFlagBits";
    }
}

/// Convert a VkResult value into the corresponding std::string_view
/// @param result The VkResult to convert
/// @return A std::string_view which contains the result
/// @note This function converts the VkResult into the corresponding std::string_view
/// If you want to convert it into an error description text, see ```result_to_description```
template <>
std::string_view as_string(const VkResult result) {
    switch (result) {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    default:
        return "Unknown VkResult";
    }
}

/// Convert the format member of a VkSurfaceFormatKHR value into the corresponding std::string_view
/// @param surface_format The surface format
/// @return A std::string_view which contains the surface format
template <>
std::string_view as_string(const VkSurfaceFormatKHR surface_format) {
    return as_string(surface_format.format);
}

/// Convert a VkSurfaceTransformFlagBitsKHR value into the corresponding std::string_view
/// @param flag The surface transform flag
/// @return A std::string_view which contains the surface transform flag
template <>
std::string_view as_string(const VkSurfaceTransformFlagBitsKHR flag) {
    switch (flag) {
    case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR";
    case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR";
    case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR";
    case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR";
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR";
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR";
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR";
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR";
    case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR:
        return "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR";
    default:
        return "Unknown VkSurfaceTransformFlagBitsKHR";
    }
}

std::string_view get_device_feature_description(const std::size_t index) {
    std::array<std::string_view, sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32)> FEATURE_DESCRIPTIONS{
        // robustBufferAccess
        "accesses to buffers which are bounds-checked against the range of the buffer descriptor",
        // fullDrawIndexUint32
        "full 32-bit range of indices for indexed draw calls when using a VkIndexType of VK_INDEX_TYPE_UINT32",
        // imageCubeArray
        "creation of image views of type VK_IMAGE_VIEW_TYPE_CUBE_ARRAY",
        // independentBlend
        "VkPipelineColorBlendAttachmentState settings which are controlled independently per-attachment",
        // geometryShader
        "geometry shaders",
        // tessellationShader
        "tessellation control and evaluation shaders",
        // sampleRateShading
        "sample shading and multisample interpolation",
        // dualSrcBlend
        "blend operations which take two sources",
        // logicOp
        "color blending logic operations",
        // multiDrawIndirect
        "multiple draw indirect",
        // drawIndirectFirstInstance
        "firstInstance parameter in VkDrawIndirectCommand",
        // depthClamp
        "depth clamping in rasterization",
        // depthBiasClamp
        "depth bias clamping in rasterization",
        // fillModeNonSolid
        "point and wireframe fill modes",
        // depthBounds
        "depth bound tests",
        // wideLines
        "lines with width other than 1.0 in rasterization",
        // largePoints
        "points with size greater than 1.0 in rasterization",
        // alphaToOne
        "replacing the alpha value of the fragment shader color output in the multisample coverage fragment operation",
        // multiViewport
        "more than one viewport",
        // samplerAnisotropy
        "anisotropic filtering",
        // textureCompressionETC2
        "all of the ETC2 and EAC compressed texture formats",
        // textureCompressionASTC_LDR
        "all of the ASTC LDR compressed texture formats",
        // textureCompressionBC
        "all of the BC compressed texture formats",
        // occlusionQueryPrecise
        "occlusion queries returning actual sample counts",
        // pipelineStatisticsQuery
        "pipeline statistics queries",
        // vertexPipelineStoresAndAtomics
        "storage buffers and images which support atomic operations in vertex, tessellation, and geometry shaders",
        // fragmentStoresAndAtomics
        "storage buffers and images which support atomic operations in fragment shaders",
        // shaderTessellationAndGeometryPointSize
        "PointSize built-in decoration being available in the tessellation control, tessellation evaluation, and "
        "geometry shaders",
        // shaderImageGatherExtended
        "extended set of image gather instructions in shader code",
        // shaderStorageImageExtendedFormats
        "all the storage image extended formats",
        // shaderStorageImageMultisample
        "multisampled storage images",
        // shaderStorageImageReadWithoutFormat
        "storage images which require a format qualifier to be specified when reading",
        // shaderStorageImageWriteWithoutFormat
        "storage images which require a format qualifier to be specified when writing",
        // shaderUniformBufferArrayDynamicIndexing
        "arrays of uniform buffers which can be indexed by dynamically uniform integer expressions in shader code",
        // shaderSampledImageArrayDynamicIndexing
        "arrays of samplers or sampled images which can be indexed by dynamically uniform integer expressions in "
        "shader code",
        // shaderStorageBufferArrayDynamicIndexing
        "arrays of storage buffers which can be indexed by dynamically uniform integer expressions in shader code",
        // shaderStorageImageArrayDynamicIndexing
        "arrays of storage images which can be indexed by dynamically uniform integer expressions in shader code",
        // shaderClipDistance
        "clip distances which are supported in shader code",
        // shaderCullDistance
        "cull distances in shader code",
        // shaderFloat64
        "64-bit floats (doubles) in shader code",
        // shaderInt64
        "64-bit integers (signed and unsigned) in shader code",
        // shaderInt16
        "16-bit integers (signed and unsigned) in shader code",
        // shaderResourceResidency
        "image operations that return resource residency information in shader code",
        // shaderResourceMinLod
        "image operations specifying the minimum resource LOD in shader code",
        // sparseBinding
        "resource memory which can be managed at opaque sparse block level instead of at the object level",
        // sparseResidencyBuffer
        "access to partially resident buffers",
        // sparseResidencyImage2D
        "access to partially resident 2D images with 1 sample per pixel",
        // sparseResidencyImage3D
        "access to partially resident 3D images",
        // sparseResidency2Samples
        "access to partially resident 2D images with 2 samples per pixel",
        // sparseResidency4Samples
        "access to partially resident 2D images with 4 samples per pixel",
        // sparseResidency8Samples
        "access to partially resident 2D images with 8 samples per pixel",
        // sparseResidency16Samples
        "access to partially resident 2D images with 16 samples per pixel",
        // sparseResidencyAliased
        "correct access to data aliased into multiple locations",
        // variableMultisampleRate
        "variable multisample rate",
        // inheritedQueries
        "execution of secondary command buffers while a query is active"};

    assert(index < FEATURE_DESCRIPTIONS.size());
    return FEATURE_DESCRIPTIONS[index];
}

template <>
VkObjectType get_vulkan_object_type(VkBuffer buf) {
    return VK_OBJECT_TYPE_BUFFER;
}

template <>
VkObjectType get_vulkan_object_type(VkCommandBuffer cmd_buf) {
    return VK_OBJECT_TYPE_COMMAND_BUFFER;
}

template <>
VkObjectType get_vulkan_object_type(VkCommandPool cmd_pool) {
    return VK_OBJECT_TYPE_COMMAND_POOL;
}

template <>
VkObjectType get_vulkan_object_type(VkInstance inst) {
    return VK_OBJECT_TYPE_INSTANCE;
}

template <>
VkObjectType get_vulkan_object_type(VkPhysicalDevice phys_device) {
    return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
}

template <>
VkObjectType get_vulkan_object_type(VkDescriptorPool desc_pool) {
    return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
}

template <>
VkObjectType get_vulkan_object_type(VkDescriptorSet descriptor_set) {
    return VK_OBJECT_TYPE_DESCRIPTOR_SET;
}

template <>
VkObjectType get_vulkan_object_type(VkDescriptorSetLayout desc_set_layout) {
    return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
}

template <>
VkObjectType get_vulkan_object_type(VkDevice device) {
    return VK_OBJECT_TYPE_DEVICE;
}

template <>
VkObjectType get_vulkan_object_type(VkEvent event) {
    return VK_OBJECT_TYPE_EVENT;
}

template <>
VkObjectType get_vulkan_object_type(VkFence fence) {
    return VK_OBJECT_TYPE_FENCE;
}

template <>
VkObjectType get_vulkan_object_type(VkImage img) {
    return VK_OBJECT_TYPE_IMAGE;
}

template <>
VkObjectType get_vulkan_object_type(VkImageView img_view) {
    return VK_OBJECT_TYPE_IMAGE_VIEW;
}

template <>
VkObjectType get_vulkan_object_type(VkPipeline pipeline) {
    return VK_OBJECT_TYPE_PIPELINE;
}

template <>
VkObjectType get_vulkan_object_type(VkPipelineCache pipeline_cache) {
    return VK_OBJECT_TYPE_PIPELINE_CACHE;
}

template <>
VkObjectType get_vulkan_object_type(VkPipelineLayout pipeline_layout) {
    return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
}

template <>
VkObjectType get_vulkan_object_type(VkQueryPool query_pool) {
    return VK_OBJECT_TYPE_QUERY_POOL;
}

template <>
VkObjectType get_vulkan_object_type(VkQueue queue) {
    return VK_OBJECT_TYPE_QUEUE;
}

template <>
VkObjectType get_vulkan_object_type(VkSampler sampler) {
    return VK_OBJECT_TYPE_SAMPLER;
}

template <>
VkObjectType get_vulkan_object_type(VkSemaphore semaphore) {
    return VK_OBJECT_TYPE_SEMAPHORE;
}

template <>
VkObjectType get_vulkan_object_type(VkShaderModule shader_module) {
    return VK_OBJECT_TYPE_SHADER_MODULE;
}

template <>
VkObjectType get_vulkan_object_type(VkSurfaceKHR surface) {
    return VK_OBJECT_TYPE_SURFACE_KHR;
}

template <>
VkObjectType get_vulkan_object_type(VkSwapchainKHR swapchain) {
    return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
}

std::string_view result_to_description(const VkResult result) {
    switch (result) {
    case VK_SUCCESS:
        return "Command successfully completed";
    case VK_NOT_READY:
        return "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
        return "An event is signaled";
    case VK_EVENT_RESET:
        return "An event is unsignaled";
    case VK_INCOMPLETE:
        return "A return array was too small for the result";
    case VK_SUBOPTIMAL_KHR:
        return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the "
               "surface successfully";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "A host memory allocation has failed";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "A device memory allocation has failed";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "Initialization of an object could not be completed for implementation-specific reasons";
    case VK_ERROR_DEVICE_LOST:
        return "The logical or physical device has been lost. See Lost Device";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "Mapping of a memory object has failed";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "A requested layer is not present or could not be loaded";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "A requested extension is not supported";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "A requested feature is not supported";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for "
               "implementation-specific reasons";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "Too many objects of the type have already been created";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "A requested format is not supported on this device";
    case VK_ERROR_FRAGMENTED_POOL:
        return "A pool allocation has failed due to fragmentation of the pool's memory. This must only be returned if "
               "no attempt to allocate host or device memory was made to accommodate the new allocation. This should "
               "be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain "
               "that the pool allocation failure was due to fragmentation";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "The requested window is already in use by Vulkan or another API in a manner which prevents it from "
               "being used again";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further "
               "presentation requests using the swapchain will fail. Applications must query the new surface "
               "properties and recreate their swapchain if they wish to continue presenting to the surface";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in "
               "a way that prevents sharing an image";
    case VK_ERROR_INVALID_SHADER_NV:
        return "One or more shaders failed to compile or link. More details are reported back to the application via "
               "VK_EXT_debug_report if enabled";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "A pool memory allocation has failed. This must only be returned if no attempt to allocate host or "
               "device memory was made to accommodate the new allocation. If the failure was definitely due to "
               "fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "An external handle is not a valid handle of the specified type";
    case VK_ERROR_FRAGMENTATION:
        return "A descriptor pool creation has failed due to fragmentation";
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        return "A buffer creation failed because the requested address is not available";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT "
               "failed as it did not have exclusive full-screen access. This may occur due to implementation-dependent "
               "reasons, outside of the application's control";
    case VK_ERROR_UNKNOWN:
        return "An unknown error has occurred; either the application has provided invalid input, or an implementation "
               "failure has occurred";
    default:
        return "Unknown VkResult";
    }
}

} // namespace inexor::vulkan_renderer::vk_tools
