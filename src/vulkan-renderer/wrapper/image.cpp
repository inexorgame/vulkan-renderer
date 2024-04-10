#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

// Constructor 1 (the most powerful constructor which exposes all parameters, rarely used)
Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
             const VmaAllocationCreateInfo &alloc_ci, const std::string &name)
    : m_device(device), m_format(img_ci.format), m_name(name) {
    if (m_name.empty()) {
        throw std::invalid_argument("Error: image name must not be empty!");
    }

    if (const auto result = vmaCreateImage(m_device.allocator(), &img_ci, &alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }
    // Assign an internal debug name to this image in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());

    // Set an internal debug name to this image using Vulkan debug utils (VK_EXT_debug_utils)
    m_device.set_debug_utils_object_name(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(m_img), m_name);

    // Fill in the image that was created and the format of the image
    auto filled_img_view_ci = img_view_ci;
    filled_img_view_ci.image = m_img;
    filled_img_view_ci.format = img_ci.format;

    if (const auto result = vkCreateImageView(m_device.device(), &filled_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }

    // Set an internal debug name to this image using Vulkan debug utils (VK_EXT_debug_utils)
    m_device.set_debug_utils_object_name(VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<std::uint64_t>(m_img_view),
                                         m_name);
}

// Constructor 2 (calls constructor 1 internally)
Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
             const std::string &name)
    : Image(device, img_ci, img_view_ci,
            {
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            },
            name) {}

// Constructor 3 (calls constructor 2 internally)
Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageAspectFlags aspect_flags,
             const std::string &name)
    : Image(device, img_ci,
            make_info<VkImageViewCreateInfo>({
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = img_ci.format,
                .subresourceRange{
                    .aspectMask = aspect_flags,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            }),
            name) {}

// Constructor 4 (calls constructor 3 internally)
Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const std::string &name)
    : Image(device, img_ci, VK_IMAGE_ASPECT_COLOR_BIT, name) {}

// Constructor 5 (calls constructor 3 (not 4!) internally)
Image::Image(const Device &device, const VkFormat format, const std::uint32_t width, const std::uint32_t height,
             const VkImageUsageFlags usage, const VkImageAspectFlags aspect_flags, const std::string &name)
    : Image(device,
            wrapper::make_info<VkImageCreateInfo>({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = format,
                .extent{
                    .width = width,
                    .height = height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            }),
            aspect_flags, name) {}

Image::Image(const Device &device, const VkFormat format, const std::uint32_t width, const std::uint32_t height,
             const VkImageUsageFlags usage, const VkImageAspectFlags aspect_flags, const std::string &name,
             VkSampleCountFlags sample_count)
    : Image(device,
            wrapper::make_info<VkImageCreateInfo>({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = format,
                .extent{
                    .width = width,
                    .height = height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = static_cast<VkSampleCountFlagBits>(sample_count),
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            }),
            aspect_flags, name) {}

Image::Image(const Device &device, const VkFormat format, const std::uint32_t width, const std::uint32_t height,
             const VkImageUsageFlags usage, const VkImageAspectFlags aspect_flags, const VkImageLayout initial_layout,
             const std::string &name)
    : Image(device,
            wrapper::make_info<VkImageCreateInfo>({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = format,
                .extent{
                    .width = width,
                    .height = height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = initial_layout,
            }),
            aspect_flags, name) {}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_format = other.m_format;
    m_alloc = other.m_alloc;
    m_alloc_info = other.m_alloc_info;
    m_img = std::exchange(other.m_img, VK_NULL_HANDLE);
    m_img_view = std::exchange(other.m_img_view, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Image::~Image() {
    // The image view must be destroyed before the image
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
}

} // namespace inexor::vulkan_renderer::wrapper
