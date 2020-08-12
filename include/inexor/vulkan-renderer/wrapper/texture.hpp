#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

// TODO: 3D textures and cube maps.
// TODO: Scan asset directory automatically.
// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

class Texture {
private:
    std::unique_ptr<wrapper::Image> m_texture_image;

    std::string m_name;
    std::string m_file_name;
    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    const wrapper::Device &m_device;
    VkSampler m_sampler;
    VmaAllocator m_vma_allocator;
    VkQueue m_data_transfer_queue;
    VkPhysicalDevice m_graphics_card;

    std::uint32_t m_data_transfer_queue_family_index;
    const VkFormat m_texture_image_format{VK_FORMAT_R8G8B8A8_UNORM};

    OnceCommandBuffer m_copy_command_buffer;

    ///
    void create_texture(void *texture_data, const std::size_t texture_size);

    ///
    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    ///
    void create_texture_sampler();

public:
    /// @brief Creates a texture from a file.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param graphics_card [in] The graphics card.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param file_name [in] The file name of the texture.
    /// @param name [in] The internal memory allocation name of the texture.
    /// @param data_transfer_queue [in] The Vulkan data transfer queue.
    /// @param data_transfer_queue_family_index [in] The queue family index of the data transfer queue to use.
    Texture(const wrapper::Device &device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
            const std::string &file_name, const std::string &name, const VkQueue data_transfer_queue,
            const std::uint32_t data_transfer_queue_family_index);

    /// @brief Creates a texture from memory.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param graphics_card [in] The graphics card.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param texture_data [in] The texture data.
    /// @param texture_size [in] The size of the texture.
    /// @param name [in] The internal memory allocation name of the texture.
    /// @param data_transfer_queue [in] The Vulkan data transfer queue.
    /// @param data_transfer_queue_family_index [in] The queue family index of the data transfer queue to use.
    Texture(const wrapper::Device &device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
            void *texture_data, const std::size_t texture_size, const std::string &name,
            const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index);
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

    [[nodiscard]] const VkImage image() const {
        assert(m_texture_image);
        return m_texture_image->get();
    }

    [[nodiscard]] const VkImageView image_view() const {
        assert(m_texture_image);
        return m_texture_image->image_view();
    }

    [[nodiscard]] const VkSampler sampler() const {
        assert(m_texture_image);
        return m_sampler;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
