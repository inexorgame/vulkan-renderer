#pragma once

#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class GPUMemoryBuffer;
class OnceCommandBuffer;

// TODO: 3D textures and cube maps.
// TODO: Scan asset directory automatically.
// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

class GpuTexture {
    std::unique_ptr<wrapper::Image> m_texture_image;
    OnceCommandBuffer m_copy_command_buffer;
    VkSampler m_sampler;

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    std::string m_name;
    const wrapper::Device &m_device;
    const VkFormat m_texture_image_format{VK_FORMAT_R8G8B8A8_UNORM};

    ///
    void create_texture(void *texture_data, const std::size_t texture_size);

    ///
    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    ///
    void create_texture_sampler();

public:
    /// @brief Loads texture data into the Gpu from a CpuTexture instance.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param cpu_texture [in] The CpuTexture instance containing the texture data.
    GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture);

    /// @brief Loads texture data into the Gpu from memory.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param data [in] The texture data.
    /// @param data_size [in] The size of the texture data.
    /// @param texture_width [in] The width of the texture.
    /// @param texture_height [in] The height of the texture.
    /// @param texture_channels [in] The number of channels in the texture.
    /// @param mip_levels [in] The number of mip levels in the texture.
    /// @param name [in] The internal memory allocation name of the texture.
    GpuTexture(const wrapper::Device &device, void *data, const std::size_t data_size, const int texture_width,
               const int texture_height, const int texture_channels, const int mip_levels, std::string name);

    GpuTexture(const GpuTexture &) = delete;
    GpuTexture(GpuTexture &&) noexcept;

    ~GpuTexture();

    GpuTexture &operator=(const GpuTexture &) = delete;
    GpuTexture &operator=(GpuTexture &&) = default;

    [[nodiscard]] std::string name() const {
        return m_name;
    }

    [[nodiscard]] VkImage image() const {
        return m_texture_image->get();
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_texture_image->image_view();
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
