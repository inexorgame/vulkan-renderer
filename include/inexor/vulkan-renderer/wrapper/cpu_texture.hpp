#pragma once

#include <string>
#include <vector>

#include <stb_image.h>

namespace inexor::vulkan_renderer::wrapper {

class CpuTexture {
    std::string m_name;

    int m_texture_width{0};
    int m_texture_height{0};
    int m_texture_channels{0};
    int m_mip_levels{0};

    stbi_uc *m_texture_data{nullptr};

    ///
    void generate_error_texture_data();

public:
    /// @brief Creates a CpuTexture instance with a default texture.
    CpuTexture();
    /// @brief Reads a texture from a file.
    /// @param file_name [in] The file name of the texture.
    /// @param name [in] The internal memory allocation name of the texture.
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

    [[nodiscard]] void *data() const {
        return static_cast<void *>(m_texture_data);
    }

    [[nodiscard]] std::size_t data_size() const {
        // TODO: We will need to update this once we fully support mip levels.
        return m_texture_width * m_texture_height * m_texture_channels;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
