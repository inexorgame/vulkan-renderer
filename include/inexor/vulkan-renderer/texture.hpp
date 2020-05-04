#pragma once

#include "inexor/vulkan-renderer/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/staging_buffer.hpp"

#include <vulkan/vulkan.h>

#include <cassert>
#include <string>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <vma/vma_usage.h>
#include <vulkan/vulkan.h>
#define TINYGLTF_NO_INCLUDE_JSON
#include <tiny_gltf/tiny_gltf.h>

namespace inexor::vulkan_renderer {

// TODO: 3D textures and cube maps.
// TODO: Scan asset directory automatically.
// TODO: Create multiple textures from file and submit them in 1 command buffer for performance reasons.

class Texture {
private:
    std::string name = "";
    std::string file_name = "";
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;
    int mip_levels = 0;

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice graphics_card = VK_NULL_HANDLE;
    VkQueue data_transfer_queue = VK_NULL_HANDLE;

    VmaAllocator vma_allocator = VK_NULL_HANDLE;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;

    VkImage image = VK_NULL_HANDLE;
    VkImageView image_view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkFormat texture_image_format = VK_FORMAT_R8G8B8A8_UNORM;

    OnceCommandBuffer copy_command_buffer;

    ///
    void transition_image_layout(VkImage &image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    ///
    void create_texture_image_view();

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
    Texture(const VkDevice &device, const VkPhysicalDevice &graphics_card, const VmaAllocator &vma_allocator, const std::string &file_name,
            const std::string &name, const VkQueue &data_transfer_queue);

    /// @brief Creates a texture from memory.
    /// @param device [in] The Vulkan device from which the texture will be created.
    /// @param graphics_card [in] The graphics card.
    /// @param vma_allocator [in] The Vulkan Memory Allocator library handle.
    /// @param texture_data [in] The texture data.
    /// @param texture_size [in] The size of the texture.
    /// @param name [in] The internal memory allocation name of the texture.
    /// @param data_transfer_queue [in] The Vulkan data transfer queue.
    Texture(const VkDevice &device, const VkPhysicalDevice &graphics_card, const VmaAllocator &vma_allocator, void *texture_data,
            const std::size_t texture_size, std::string &name, const VkQueue &data_transfer_queue);

    ~Texture();

    const std::string &get_name() const {
        return name;
    }

    const std::string &get_file_name() const {
        return file_name;
    }

    const VkImage get_image() const {
        return image;
    }

    const VkImageView get_image_view() const {
        return image_view;
    }

    const VkSampler get_sampler() const {
        return sampler;
    }
};

} // namespace inexor::vulkan_renderer
