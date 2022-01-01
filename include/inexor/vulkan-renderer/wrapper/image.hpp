#pragma once

#include <vk_mem_alloc.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

class Image {
private:
    const Device &m_device;

    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};

    VkImage m_image{VK_NULL_HANDLE};
    VkImageView m_image_view{VK_NULL_HANDLE};
    VkSampler m_sampler{VK_NULL_HANDLE};

    VkImageLayout m_image_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;
    VkSamplerCreateInfo m_sampler_ci;
    VkDescriptorImageInfo m_descriptor;

    std::string m_name;

    void create_image();
    void create_image_view();
    void create_sampler();
    void update_descriptor();

    /*
    VkSamplerCreateInfo default_texture_sampler(const wrapper::Device &device) {
    auto sampler_ci = wrapper::make_info<VkSamplerCreateInfo>();

    sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.compareEnable = VK_FALSE;
    sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    // Check if anisotropic filtering is available before we enable its use

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device.physical_device(), &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(device.physical_device(), &graphics_card_properties);

    if (device_features.samplerAnisotropy) {
        sampler_ci.maxAnisotropy = graphics_card_properties.limits.maxSamplerAnisotropy;
        sampler_ci.anisotropyEnable = VK_TRUE;
    } else {
        sampler_ci.maxAnisotropy = 1.0;
        sampler_ci.anisotropyEnable = VK_FALSE;
    }

    return sampler_ci;
}
    */

public:
    ///
    ///
    ///
    ///
    ///
    ///
    Image(const Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci,
          VkSamplerCreateInfo sampler_ci, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;

    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) noexcept = default;

    ///
    ///
    ///
    void transition_image_layout(VkCommandBuffer cmd_buf, VkImageLayout new_layout, std::uint32_t miplevel_count = 1,
                                 std::uint32_t layer_count = 1);

    ///
    ///
    ///
    void copy_from_buffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, std::uint32_t width,
                          std::uint32_t height);

    ///
    ///
    ///
    void copy_from_image(VkCommandBuffer command_buffer, Image &image, std::uint32_t width, std::uint32_t height,
                         std::uint32_t miplevel_count, std::uint32_t layer_count, std::uint32_t base_array_layer,
                         std::uint32_t mip_level);

    [[nodiscard]] VkFormat format() const {
        return m_image_ci.format;
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }

    // TODO: Add get methods for all members of m_image_ci.

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_image;
    }

    [[nodiscard]] VkImageLayout image_layout() const {
        return m_image_layout;
    }

    [[nodiscard]] VkDescriptorImageInfo descriptor() const {
        return m_descriptor;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
