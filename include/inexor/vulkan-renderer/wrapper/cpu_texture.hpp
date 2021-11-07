#pragma once

#include "inexor/vulkan-renderer/exception.hpp"

#include "inexor/vulkan-renderer/wrapper/texture_attributes.hpp"

#include <cassert>
#include <string>
#include <vector>

#include <stb_image.h>

#include <vulkan/vulkan.h>

#include <ktxvulkan.h>

namespace inexor::vulkan_renderer::wrapper {

/// @brief RAII wrapper class for texture data.
/// TODO: Scan asset directory automatically.
class CpuTexture {
private:
    TextureAttributes m_attributes;

    stbi_uc *m_texture_data{nullptr};

    // Khronos texture format library (ktx)
    ktxTexture *m_ktx_texture{nullptr};
    ktx_uint8_t *m_ktx_texture_data{nullptr};
    ktx_size_t m_ktx_texture_data_size{0};

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
    CpuTexture(const std::string &file_name, std::string name);

    CpuTexture(const CpuTexture &) = delete;
    CpuTexture(CpuTexture &&) noexcept;

    ~CpuTexture();

    CpuTexture &operator=(const CpuTexture &) = delete;
    CpuTexture &operator=(CpuTexture &&) = default;

    [[nodiscard]] const std::string &name() const {
        return m_attributes.name;
    }

    [[nodiscard]] std::uint32_t width() const {
        return m_attributes.width;
    }

    [[nodiscard]] std::uint32_t height() const {
        return m_attributes.height;
    }

    [[nodiscard]] std::uint32_t channels() const {
        return m_attributes.channels;
    }

    [[nodiscard]] std::uint32_t mip_levels() const {
        return m_attributes.mip_levels;
    }

    [[nodiscard]] TextureAttributes attributes() const {
        return m_attributes;
    }

    [[nodiscard]] const void *data() const {
        // The texture data is either stored in stb_image or ktx wrapper
        return (m_ktx_texture) ? m_ktx_texture_data : m_texture_data;
    }

    [[nodiscard]] std::size_t data_size() const {
        std::size_t data_size = 0;

        // TODO: Is this correct now with mip level support?

        if (m_texture_data) {
            data_size = static_cast<std::size_t>(m_attributes.width) * static_cast<std::size_t>(m_attributes.height) *
                        static_cast<std::size_t>(m_attributes.channels);
        } else if (m_ktx_texture_data) {
            data_size = m_ktx_texture_data_size;
        } else {
            throw std::runtime_error("Error: texture data size invalid!");
        }

        return data_size;
    }

    [[nodiscard]] const auto ktx_wrapper() const {
        return m_ktx_texture;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
