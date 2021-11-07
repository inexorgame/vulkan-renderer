#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"

#include "inexor/vulkan-renderer/tools/file.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <array>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

CpuTexture::CpuTexture() {
    m_attributes.name = "Unknown texture";
    generate_error_texture_data();
}

void CpuTexture::load_ktx_texture(const std::string &file_name) {
    assert(!file_name.empty());

    spdlog::trace("Loading ktx texture {}", file_name);

    const auto result =
        ktxTexture_CreateFromNamedFile(file_name.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &m_ktx_texture);

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("Error: ktxTexture_CreateFromNamedFile failed for file " + file_name + "!");
    }

    m_attributes.width = m_ktx_texture->baseWidth;
    m_attributes.height = m_ktx_texture->baseHeight;
    m_attributes.mip_levels = m_ktx_texture->numLevels;

    m_ktx_texture_data = ktxTexture_GetData(m_ktx_texture);

    if (!m_ktx_texture_data) {
        throw std::runtime_error("Error: ktx texture data is invalid!");
    }

    m_ktx_texture_data_size = ktxTexture_GetDataSize(m_ktx_texture);

    if (m_ktx_texture_data_size == 0) {
        throw std::runtime_error("Error: ktx texture data size is zero!");
    }
}

void CpuTexture::load_texture(const std::string &file_name) {
    int width = 0;
    int height = 0;

    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    m_texture_data = stbi_load(file_name.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

    m_attributes.width = width;
    m_attributes.height = height;

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
        m_attributes.channels = 4;

        // TODO: We are currently only supporting 1 mip level.
        m_attributes.mip_levels = 1;

        spdlog::debug("Texture dimensions: width: {}, height: {}, channels: {} mip levels: {}.", m_attributes.width,
                      m_attributes.height, m_attributes.channels, m_attributes.mip_levels);
    }
}

CpuTexture::CpuTexture(const std::string &file_name, std::string name) {
    assert(!file_name.empty());
    assert(!name.empty());

    m_attributes.name = name;

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
        load_ktx_texture(file_name);
    } else {
        throw std::runtime_error("Error: Unsupported texture file extension " + file_extension + "!");
    }
}

CpuTexture::CpuTexture(CpuTexture &&other) noexcept {
    m_attributes = std::move(other.m_attributes);
    m_texture_data = other.m_texture_data;
    m_texture_data = std::exchange(other.m_texture_data, nullptr);
    m_ktx_texture = std::exchange(other.m_ktx_texture, nullptr);
    m_ktx_texture_data = std::exchange(other.m_ktx_texture_data, nullptr);
    m_ktx_texture_data_size = other.m_ktx_texture_data_size;
}

CpuTexture::~CpuTexture() {
    stbi_image_free(m_texture_data);

    if (m_ktx_texture) {
        ktxTexture_Destroy(m_ktx_texture);
    }
}

void CpuTexture::generate_error_texture_data() {
    assert(m_texture_data == nullptr);

    m_attributes.width = 512;
    m_attributes.height = 512;
    m_attributes.channels = 4;
    m_attributes.mip_levels = 1;

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
    for (std::uint32_t y = 0; y < m_attributes.height; y++) {
        for (std::uint32_t x = 0; x < m_attributes.width; x++) {
            const int color_id = get_color(x, y, SQUARE_DIMENSION, COLORS.size());
            std::memcpy(m_texture_data, &COLORS[color_id][0], 4 * sizeof(COLORS[color_id][0]));
        }
    }
}

} // namespace inexor::vulkan_renderer::wrapper
