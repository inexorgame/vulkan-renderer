#pragma once

#include "inexor/vulkan-renderer/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer {

// TODO: 3D textures and cube maps.
// TODO: Scan asset directory automatically.
// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

class Texture {
private:
    std::unique_ptr<wrapper::Image> texture_image;

    std::string name = "";
    std::string file_name = "";
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;
    int mip_levels = 0;

    VkDevice device;
    VkSampler sampler;
    VmaAllocator vma_allocator;
    VkQueue data_transfer_queue;
    VkPhysicalDevice graphics_card;

    std::uint32_t data_transfer_queue_family_index;
    const VkFormat texture_image_format = VK_FORMAT_R8G8B8A8_UNORM;

    OnceCommandBuffer copy_command_buffer;

    ///
    void create_texture(void *texture_data, const std::size_t texture_size);

    ///
    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    ///
    void create_texture_sampler();

public:
    /// Delete the copy constructor so textures are move-only objects.
    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;

    /// Delete the copy assignment operator so shaders are move-only objects.
    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) noexcept = default;

    /// @brief Creates a texture from a file.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param graphics_card [in] The graphics card.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param file_name [in] The file name of the texture.
    /// @param name [in] The internal memory allocation name of the texture.
    /// @param data_transfer_queue [in] The Vulkan data transfer queue.
    /// @param data_transfer_queue_family_index [in] The queue family index of the data transfer queue to use.
    Texture(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
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
    Texture(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
            void *texture_data, const std::size_t texture_size, const std::string &name,
            const VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index);

    ~Texture();

    [[nodiscard]] const std::string &get_name() const {
        return name;
    }

    [[nodiscard]] const std::string &get_file_name() const {
        return file_name;
    }

    [[nodiscard]] const VkImage get_image() const {
        assert(texture_image);
        return texture_image->get();
    }

    [[nodiscard]] const VkImageView get_image_view() const {
        assert(texture_image);
        return texture_image->get_image_view();
    }

    [[nodiscard]] const VkSampler get_sampler() const {
        assert(texture_image);
        return sampler;
    }
};

} // namespace inexor::vulkan_renderer
