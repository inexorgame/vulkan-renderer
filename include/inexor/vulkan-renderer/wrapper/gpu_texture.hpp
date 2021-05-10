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

/// @note The code which loads textures from files is wrapped in CpuTexture.
/// @brief RAII wrapper class for textures which are stored in GPU memory.
/// @todo Support 3D textures and cube maps (implement new and separate wrappers though).
class GpuTexture {
    std::unique_ptr<wrapper::Image> m_texture_image;
    OnceCommandBuffer m_copy_command_buffer;
    VkSampler m_sampler{VK_NULL_HANDLE};

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    std::string m_name;
    const wrapper::Device &m_device;

    // TODO: Expose this as parameter and support other texture formats as well?
    const VkFormat m_texture_image_format{VK_FORMAT_R8G8B8A8_UNORM};

    /// @brief Create the texture.
    /// @param texture_data A pointer to the texture data.
    /// @param texture_size The size of the texture.
    void create_texture(void *texture_data, std::size_t texture_size);

    /// @brief Transform the image layout.
    /// @param image The image.
    /// @param old_layout The old image layout.
    /// @param new_layout The new image layout.
    void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

    /// @brief Create the texture sampler.
    void create_texture_sampler();

public:
    /// @brief Construct a texture from a file.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param file_name The name of the texture file.
    /// @param name The internal debug marker name of the texture.
    GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture);

    /// @brief Construct a texture from a block of memory.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param texture_data A pointer to the texture data.
    /// @param texture_width The width of the texture.
    /// @param texture_height The height of the texture.
    /// @param texture_size The size of the texture.
    /// @param name The internal debug marker name of the texture.
    GpuTexture(const wrapper::Device &device, void *data, std::size_t data_size, int texture_width, int texture_height,
               int texture_channels, int mip_levels, std::string name);

    GpuTexture(const GpuTexture &) = delete;
    GpuTexture(GpuTexture &&) noexcept;

    ~GpuTexture();

    GpuTexture &operator=(const GpuTexture &) = delete;
    GpuTexture &operator=(GpuTexture &&) = delete;

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
