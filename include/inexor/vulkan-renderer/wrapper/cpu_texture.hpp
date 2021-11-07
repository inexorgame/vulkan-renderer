#pragma once

#include <string>
#include <vector>

#include <stb_image.h>

namespace inexor::vulkan_renderer::wrapper {

/// @brief RAII wrapper class for texture data.
/// TODO: Scan asset directory automatically.
class CpuTexture {
private:
    std::string m_name;

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    stbi_uc *m_texture_data{nullptr};

    void load_texture(const std::string &file_name);

    void load_ktx_texture(const std::string &file_name);

    /// @brief Generate a chessboard color pattern which will be used as error texture.
    void generate_error_texture_data();

public:
    /// @brief Create a CpuTexture instance with a default texture.
    CpuTexture();

    /// @brief Read a texture from a file.
    /// @param file_name The file name of the texture.
    /// @param name The internal debug marker name of the command buffer. This must not be an empty string.
    CpuTexture(const std::string &file_name, std::string name);

    CpuTexture(const CpuTexture &) = delete;
    CpuTexture(CpuTexture &&) noexcept;

    ~CpuTexture();

    CpuTexture &operator=(const CpuTexture &) = delete;
    CpuTexture &operator=(CpuTexture &&) = default;

    [[nodiscard]] std::string name() const {
        return m_name;
    }

    [[nodiscard]] int width() const {
        return m_texture_width;
    }

    [[nodiscard]] int height() const {
        return m_texture_height;
    }

    [[nodiscard]] int channels() const {
        return m_texture_channels;
    }

    [[nodiscard]] int mip_levels() const {
        return m_mip_levels;
    }

    [[nodiscard]] stbi_uc *data() const {
        return m_texture_data;
    }

    [[nodiscard]] std::size_t data_size() const {
        // TODO: We will need to update this once we fully support mip levels.
        return static_cast<std::size_t>(m_texture_width) * static_cast<std::size_t>(m_texture_height) *
               static_cast<std::size_t>(m_texture_channels);
    }
};

} // namespace inexor::vulkan_renderer::wrapper
