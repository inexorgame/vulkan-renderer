#pragma once

#include "inexor/vulkan-renderer/exception.hpp"

#include <ktxvulkan.h>
#include <stb_image.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::texture {

/// @brief RAII wrapper class for texture data.
/// TODO: Scan asset directory automatically.
class CpuTexture {
private:
    // The stb_image texture data
    stbi_uc *m_texture_data{nullptr};

    // Khronos texture format library (ktx)
    ktxTexture *m_ktx_texture{nullptr};
    ktx_uint8_t *m_ktx_texture_data{nullptr};
    ktx_size_t m_ktx_texture_data_size{0};

    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::uint32_t m_channels{4};
    std::uint32_t m_mip_levels{1};
    std::string m_name;

    void load_texture(const std::string &file_name);

    void load_ktx_texture(const std::string &file_name);

    /// @brief Generate a chessboard color pattern which will be used as error texture.
    void generate_error_texture_data();

public:
    /// @brief Create a CpuTexture instance with a default texture.
    CpuTexture();

    /// @brief Read a texture from a file.
    /// @param file_name The file name of the texture.
    /// @param name The internal debug marker name of the texture (must not be empty).
    CpuTexture(std::string file_name, std::string name);

    CpuTexture(const CpuTexture &) = delete;
    CpuTexture(CpuTexture &&) noexcept;

    ~CpuTexture();

    CpuTexture &operator=(const CpuTexture &) = delete;
    CpuTexture &operator=(CpuTexture &&) noexcept = default;

    [[nodiscard]] const void *data() const {
        // The texture data is either stored in stb_image or ktx wrapper
        return (m_ktx_texture) ? m_ktx_texture_data : m_texture_data;
    }

    [[nodiscard]] std::size_t data_size() const {
        if (m_texture_data) {
            return static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) *
                   static_cast<std::size_t>(m_channels);
        } else if (m_ktx_texture_data) {
            return m_ktx_texture_data_size;
        } else {
            return static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) *
                   static_cast<std::size_t>(m_channels);
        }
        return 0;
    }

    [[nodiscard]] std::uint32_t width() const {
        return m_width;
    }

    [[nodiscard]] std::uint32_t height() const {
        return m_height;
    }

    [[nodiscard]] std::uint32_t channels() const {
        return m_channels;
    }

    [[nodiscard]] std::uint32_t miplevel_count() const {
        return m_mip_levels;
    }

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] const auto ktx_wrapper() const {
        return m_ktx_texture;
    }

    [[nodiscard]] const auto ktx_texture_data_size() const {
        return m_ktx_texture_data_size;
    }

    [[nodiscard]] const auto ktx_texture_data() const {
        return m_ktx_texture_data;
    }
};

} // namespace inexor::vulkan_renderer::texture
