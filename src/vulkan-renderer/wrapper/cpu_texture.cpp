#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"

#include "inexor/vulkan-renderer/tools/file.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>
#include <stb_image.h>

// TODO: Can we include only vulkan_core.hpp?
#include <vulkan/vulkan.h>

// Khronos texture format (ktx)
#include <ktxvulkan.h>

#include <array>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

CpuTexture::CpuTexture() : m_name("default texture") {
    generate_error_texture_data();
}

void CpuTexture::load_ktx_texture(const std::string &file_name) {
    assert(!file_name.empty());

    spdlog::trace("Loading ktx texture {}", file_name);

    ktxTexture *ktxTexture;

    const auto result =
        ktxTexture_CreateFromNamedFile(file_name.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("Error: ktxTexture_CreateFromNamedFile failed for file " + file_name + "!");
    }

    // TODO: continue...

    ktxTexture_Destroy(ktxTexture);
}

void CpuTexture::load_texture(const std::string &file_name) {
    // Load the texture file using stb_image library.
    // Force stb_image to load an alpha channel as well.
    m_texture_data = stbi_load(file_name.c_str(), &m_texture_width, &m_texture_height, nullptr, STBI_rgb_alpha);

    if (m_texture_data == nullptr) {
        spdlog::error("Could not load texture file {} using stbi_load!  Falling back to error texture.", file_name);
        generate_error_texture_data();
    } else {

        // TODO: We are currently hard coding the number of channels with STBI_rgb_alpha.
        //       Eventually, we probably need to pass this information into this class from
        //       a higher level class - like a material loader class.
        //       So, as an example, if the material loader is loading a normal map, we know
        //       we need to tell this class to load a 3 channel texture.  If it can, great.
        //       If it can not, then this class probably needs to load a 3 channel error texture.
        m_texture_channels = 4;

        // TODO: We are currently only supporting 1 mip level.
        m_mip_levels = 1;

        spdlog::debug("Texture dimensions: width: {}, height: {}, channels: {} mip levels: {}.", m_texture_width,
                      m_texture_height, m_texture_channels, m_mip_levels);
    }
}

CpuTexture::CpuTexture(const std::string &file_name, std::string name) : m_name(std::move(name)) {
    assert(!file_name.empty());
    assert(!m_name.empty());

    spdlog::trace("Loading texture file {}", file_name);

    const auto file_extension = tools::get_file_extension_lowercase(file_name);

    const std::vector<std::string> allowed_formats = {"jpg", "jpeg", "png", "hdr", "gif", "bmp"};

    // Khronos texture format (ktx)
    if (file_extension == "ktx") {
        // TODO: Implement
    } else if (std::find(allowed_formats.begin(), allowed_formats.end(), file_extension) != allowed_formats.end()) {
        load_texture(file_name);
    } else {
        throw std::runtime_error("Error: Unsupported texture format " + file_extension + "!");
    }
}

CpuTexture::CpuTexture(CpuTexture &&other) noexcept {
    m_name = std::move(other.m_name);
    m_texture_width = other.m_texture_width;
    m_texture_height = other.m_texture_height;
    m_texture_channels = other.m_texture_channels;
    m_mip_levels = other.m_mip_levels;
    m_texture_data = other.m_texture_data;
}

CpuTexture::~CpuTexture() {
    stbi_image_free(m_texture_data);
}

void CpuTexture::generate_error_texture_data() {
    assert(m_texture_data == nullptr);

    m_texture_width = 512;
    m_texture_height = 512;
    m_texture_channels = 4;
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
    for (int y = 0; y < m_texture_height; y++) {
        for (int x = 0; x < m_texture_width; x++) {
            const int color_id = get_color(x, y, SQUARE_DIMENSION, COLORS.size());
            std::memcpy(m_texture_data, &COLORS[color_id][0], 4 * sizeof(COLORS[color_id][0]));
        }
    }
}

} // namespace inexor::vulkan_renderer::wrapper
