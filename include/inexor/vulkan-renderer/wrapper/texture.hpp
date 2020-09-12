#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class OnceCommandBuffer;

// TODO: 3D textures and cube maps.
// TODO: Scan asset directory automatically.
// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

/// @class Texture
/// @brief RAII wrapper class for textures.
class Texture {
    std::unique_ptr<wrapper::Image> m_texture_image;
    OnceCommandBuffer m_copy_command_buffer;
    VkSampler m_sampler;

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    const std::string m_name;
    const std::string m_file_name;
    const wrapper::Device &m_device;
    const VkFormat m_texture_image_format{VK_FORMAT_R8G8B8A8_UNORM};

    /// @brief Creates the texture.
    /// @param texture_data [in] A pointer to the texture data.
    /// @param texture_size [in] The size of the texture.
    void create_texture(void *texture_data, const std::size_t texture_size);

    /// @brief Transforms the image layout.
    /// @param image [in] The image.
    /// @param format [in] The format.
    /// @param old_layout [in] The old image layout.
    /// @param new_layout [in] The new image layout.
    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    /// @brief Create the texture sampler.
    void create_texture_sampler();

public:
    /// @brief Constructs a texture from a file.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param file_name [in] The name of the texture file.
    /// @param name [in] The internal debug marker name of the texture.
    Texture(const Device &device, const std::string &file_name, const std::string &name);

    /// @brief Constructs a texture from a block of memory.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param texture_data [in] A pointer to the texture data.
    /// @param texture_width [in] The width of the texture.
    /// @param texture_height [in] The height of the texture.
    /// @param texture_size [in] The size of the texture.
    /// @param name [in] The internal debug marker name of the texture.
    Texture(const Device &device, void *texture_data, const std::uint32_t texture_width,
            const std::uint32_t texture_height, const std::size_t texture_size, const std::string &name);

    Texture(const Texture &) = delete;
    Texture(Texture &&) noexcept;

    ~Texture();

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = default;

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] const std::string &file_name() const {
        return m_file_name;
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
