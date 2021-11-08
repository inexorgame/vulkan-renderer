#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/texture_attributes.hpp"

#include <memory>

namespace inexor::vulkan_renderer::wrapper {

///
[[nodiscard]] VkSamplerCreateInfo default_sampler_settings(VkPhysicalDevice gpu);

/// A base class for 2D textures and cubemaps
/// TODO: Implement texture arrays (3D textures)
class TextureBase {
protected:
    const Device &m_device;
    std::unique_ptr<Image> m_image;
    VkSampler m_sampler{VK_NULL_HANDLE};
    VkDescriptorImageInfo m_descriptor;
    TextureAttributes m_attributes;

    /// @brief Create the texture's VkImage and VkImageView.
    ///
    ///
    void create_image(const void *texture_data, std::size_t texture_size);

    void create_texture_sampler(VkSamplerCreateInfo sampler_ci);

    void create_default_texture_sampler();

    void update_descriptor();

public:
    // TODO: How many fields should actually be set through this constructor?
    TextureBase(const wrapper::Device &device);

    [[nodiscard]] VkImage image() const {
        return m_image->image();
    }

    [[nodiscard]] auto &image_wrapper() const {
        return m_image;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image->image_view();
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }

    [[nodiscard]] auto &descriptor() const {
        return m_descriptor;
    }

    [[nodiscard]] const auto &attributes() const {
        return m_attributes;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
