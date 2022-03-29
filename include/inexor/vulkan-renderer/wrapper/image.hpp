#pragma once

#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

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
    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;

    std::string m_name;

    void create_image();
    void create_image_view();

public:
    // TODO: Make private member and add get methods!
    VkDescriptorImageInfo descriptor_image_info;

    Image(const Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;

    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) noexcept = default;

    // Execute the image layout transition by taking an external command buffer
    void change_image_layout(const wrapper::CommandBuffer &cmd_buf, VkImageLayout new_layout,
                             std::uint32_t miplevel_count = 1, std::uint32_t layer_count = 1,
                             std::uint32_t base_mip_level = 0, std::uint32_t base_array_layer = 0);

    ///
    ///
    ///
    void copy_from_buffer(const wrapper::CommandBuffer &cmd_buf, VkBuffer src_buffer, std::uint32_t width,
                          std::uint32_t height);

    ///
    ///
    ///
    void copy_from_image(const wrapper::CommandBuffer &cmd_buf, Image &image, std::uint32_t width, std::uint32_t height,
                         std::uint32_t miplevel_count, std::uint32_t layer_count, std::uint32_t base_array_layer,
                         std::uint32_t mip_level);

    [[nodiscard]] VkFormat format() const {
        return m_image_ci.format;
    }

    // TODO: Add get methods for all members of m_image_ci.

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_image;
    }

    [[nodiscard]] VkImageLayout image_layout() const {
        return descriptor_image_info.imageLayout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
