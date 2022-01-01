#include "inexor/vulkan-renderer/texture/cpu_texture.hpp"

#include "inexor/vulkan-renderer/tools/file.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <array>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::texture {

CpuTexture::CpuTexture() {
    generate_error_texture_data();
}

void CpuTexture::load_ktx_texture(const std::string &file_name) {
    assert(!file_name.empty());

    spdlog::trace("Loading ktx texture {}", file_name);

    if (const auto result = ktxTexture_CreateFromNamedFile(file_name.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                                           &m_ktx_texture) != KTX_SUCCESS) {
        throw std::runtime_error("Error: ktxTexture_CreateFromNamedFile failed for file " + file_name + "!");
    }

    m_width = m_ktx_texture->baseWidth;
    m_height = m_ktx_texture->baseHeight;
    m_mip_levels = m_ktx_texture->numLevels;

    m_ktx_texture_data = ktxTexture_GetData(m_ktx_texture);

    if (!m_ktx_texture_data) {
        // TODO: Should we throw an exception here or generate an error texture instead?
        throw std::runtime_error("Error: ktxTexture_GetData returned invalid data!");
    }

    m_ktx_texture_data_size = ktxTexture_GetDataSize(m_ktx_texture);

    if (m_ktx_texture_data_size == 0) {
        // TODO: Should we throw an exception here or generate an error texture instead?
        throw std::runtime_error("Error: ktx texture data size is zero!");
    }
}

void CpuTexture::load_texture(const std::string &file_name) {
    int width = 0;
    int height = 0;

    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    m_texture_data = stbi_load(file_name.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

    m_width = width;
    m_height = height;

    if (m_texture_data == nullptr) {
        spdlog::error("Could not load texture file {} using stbi_load! Falling back to error texture.", file_name);
        generate_error_texture_data();
    } else {

        // TODO: We are currently hard coding the number of channels with STBI_rgb_alpha.
        //       Eventually, we probably need to pass this information into this class from
        //       a higher level class - like a material loader class.
        //       So, as an example, if the material loader is loading a normal map, we know
        //       we need to tell this class to load a 3 channel texture.  If it can, great.
        //       If it can not, then this class probably needs to load a 3 channel error texture.
        m_channels = 4;

        // TODO: We are currently only supporting 1 mip level.
        m_mip_levels = 1;

        spdlog::debug("Texture dimensions: width: {}, height: {}, channels: {} mip levels: {}.", m_width, m_height,
                      m_channels, m_mip_levels);
    }
}

CpuTexture::CpuTexture(std::string file_name, std::string name) {
    assert(!file_name.empty());
    assert(!name.empty());

    m_name = name;

    spdlog::trace("Loading texture file {}", file_name);

    const auto file_extension = tools::get_file_extension_lowercase(file_name);

    // All supported formats excluding ktx
    const std::vector<std::string> default_formats = {"jpg", "jpeg", "png", "hdr", "gif", "bmp"};

    // Use the file extension to decide which third party library to use for loading the texture
    // Feel free to add more custom texture formats here
    if (std::find(default_formats.begin(), default_formats.end(), file_extension) != default_formats.end()) {
        // stb_image can load all these formats
        load_texture(file_name);
    } else if (file_extension == "ktx") {
        // Khronos texture format library
        // The library stb_image can't load this for us! We need to load these textures using a different library.
        load_ktx_texture(file_name);
    } else {
        throw std::runtime_error("Error: Unsupported texture file extension " + file_extension + "!");
    }
}

CpuTexture::CpuTexture(CpuTexture &&other) noexcept {
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_mip_levels = other.m_mip_levels;
    m_name = std::move(other.m_name);
    m_texture_data = other.m_texture_data;
    m_texture_data = std::exchange(other.m_texture_data, nullptr);
    m_ktx_texture = std::exchange(other.m_ktx_texture, nullptr);
    m_ktx_texture_data = std::exchange(other.m_ktx_texture_data, nullptr);
    m_ktx_texture_data_size = other.m_ktx_texture_data_size;
}

CpuTexture::~CpuTexture() {
    stbi_image_free(m_texture_data);

    if (m_ktx_texture != nullptr) {
        ktxTexture_Destroy(m_ktx_texture);
    }
}

void CpuTexture::generate_error_texture_data() {
    assert(m_texture_data == nullptr);

    // TODO: Move them to default settings?
    m_name = "Unknown texture";
    m_width = 512;
    m_height = 512;
    m_channels = 4;
    m_mip_levels = 1;

    // Create an 8x8 checkerboard pattern of squares.
    constexpr int SQUARE_DIMENSION{64};

    // pink, purple
    constexpr std::array<std::array<unsigned char, 4>, 2> COLORS{{{0xFF, 0x69, 0xB4, 0xFF}, {0x94, 0x00, 0xD3, 0xFF}}};

    const auto get_color = [](int x, int y, int square_dimension, std::size_t colors) -> int {
        return static_cast<int>(
            (static_cast<std::size_t>(x / square_dimension) + static_cast<std::size_t>(y / square_dimension)) % colors);
    };

    // Note: Using the stb library function since we are freeing with stbi_image_free.
    m_texture_data = static_cast<stbi_uc *>(STBI_MALLOC(data_size())); // NOLINT

    // Performance could be improved by copying complete rows after one or two rows are created with the loops.
    for (std::uint32_t y = 0; y < m_height; y++) {
        for (std::uint32_t x = 0; x < m_width; x++) {
            const int color_id = get_color(x, y, SQUARE_DIMENSION, COLORS.size());
            std::memcpy(m_texture_data, &COLORS[color_id][0], 4 * sizeof(COLORS[color_id][0]));
        }
    }
}

} // namespace inexor::vulkan_renderer::texture
