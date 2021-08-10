#pragma once

#include "inexor/vulkan-renderer/gltf/texture_sampler.hpp"
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
    std::unique_ptr<Image> m_texture_image;
    OnceCommandBuffer m_copy_command_buffer;
    VkSampler m_sampler{VK_NULL_HANDLE};

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    std::string m_name;
    const Device &m_device;

    // TODO: Expose this as parameter and support other texture formats as well?
    const VkFormat m_texture_image_format{VK_FORMAT_R8G8B8A8_UNORM};

    /// @brief Create the texture.
    /// @param texture_data A pointer to the texture data.
    /// @param texture_size The size of the texture.
    void create_texture(const void *texture_data, std::size_t texture_size);

    /// @brief Transform the image layout.
    /// @param image The image.
    /// @param old_layout The old image layout.
    /// @param new_layout The new image layout.
    void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

    /// @brief Create the texture sampler.
    void create_texture_sampler();

    /// @brief Create the texture sampler.
    void create_texture_sampler(const gltf::TextureSampler &sampler);

public:
    /// @brief Construct a texture from a file.
    /// @param device The const reference to a device RAII wrapper instance
    /// @param cpu_texture A const reference to the CPU texture
    GpuTexture(const Device &device, const CpuTexture &cpu_texture);

    /// @brief Construct a texture from a file.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param sampler
    /// @param cpu_texture
    GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const CpuTexture &cpu_texture);

    /// @brief Construct a texture from a block of memory.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param texture_data A pointer to the texture data.
    /// @param texture_width The width of the texture.
    /// @param texture_height The height of the texture.
    /// @param texture_size The size of the texture.
    /// @param name The internal debug marker name of the texture.
    GpuTexture(const Device &device, const void *data, std::size_t data_size, std::uint32_t texture_width,
               std::uint32_t texture_height, std::uint32_t texture_channels, std::uint32_t mip_levels,
               std::string name);

    /// @brief Construct a texture from a block of memory for a glTF2 model file texture.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param sampler The texture sampler for the glTF2 model.
    /// @param texture_data A pointer to the texture data.
    /// @param texture_width The width of the texture.
    /// @param texture_height The height of the texture.
    /// @param texture_size The size of the texture.
    /// @param name The internal debug marker name of the texture.
    GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const void *data, std::size_t data_size,
               std::uint32_t texture_width, std::uint32_t texture_height, std::uint32_t texture_channels,
               std::uint32_t mip_levels, std::string name);

    GpuTexture(const GpuTexture &) = delete;
    GpuTexture(GpuTexture &&) noexcept;
    ~GpuTexture();

    GpuTexture &operator=(const GpuTexture &) = delete;
    GpuTexture &operator=(GpuTexture &&) = delete;

    [[nodiscard]] const std::string &name() const {
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
