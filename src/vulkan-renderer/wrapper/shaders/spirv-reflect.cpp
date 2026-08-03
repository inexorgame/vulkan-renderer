#include "inexor/vulkan-renderer/wrapper/shaders/spirv-reflect.hpp"

#include "inexor/vulkan-renderer/tools/file.hpp"

#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::uint32_t SPIRV_MAGIC = 0x07230203u;
constexpr std::uint16_t OPCODE_ENTRY_POINT = 15u;

VkShaderStageFlagBits execution_model_to_shader_stage(const std::uint32_t execution_model) {
    switch (execution_model) {
    case 0:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case 1:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case 2:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case 3:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case 4:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case 5:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        throw std::runtime_error("Failed to parse SPIR-V: unsupported execution model.");
    }
}

std::string read_spirv_string(const std::uint32_t *words, const std::size_t word_count) {
    const auto *bytes = reinterpret_cast<const char *>(words);
    const auto byte_count = word_count * sizeof(std::uint32_t);
    std::string result;
    for (std::size_t i = 0; i < byte_count; ++i) {
        if (bytes[i] == '\0') {
            break;
        }
        result.push_back(bytes[i]);
    }
    return result;
}

} // namespace

namespace inexor::vulkan_renderer::wrapper::shaders {

VkShaderStageFlagBits get_shader_stage(const std::filesystem::path &path) {
    const auto buffer = tools::read_file_binary_data(path.string());
    const auto spirv = std::span<const char>(buffer.data(), buffer.size());
    return get_shader_stage(spirv);
}

VkShaderStageFlagBits get_shader_stage(std::span<const char> spirv) {
    if (spirv.size() < 5 * sizeof(std::uint32_t)) {
        throw std::runtime_error("Failed to parse SPIR-V: file too small.");
    }
    if (spirv.size() % sizeof(std::uint32_t) != 0) {
        throw std::runtime_error("Failed to parse SPIR-V: byte size is not aligned to 32-bit words.");
    }

    std::vector<std::uint32_t> spirv_words(spirv.size() / sizeof(std::uint32_t));
    std::memcpy(spirv_words.data(), spirv.data(), spirv.size());

    if (spirv_words[0] != SPIRV_MAGIC) {
        throw std::runtime_error("Failed to parse SPIR-V: invalid magic number.");
    }

    bool found_any_entry_point = false;
    std::optional<VkShaderStageFlagBits> fallback_stage;

    for (std::size_t index = 5; index < spirv_words.size();) {
        const auto instruction = spirv_words[index];
        const auto word_count = static_cast<std::size_t>(instruction >> 16u);
        const auto opcode = static_cast<std::uint16_t>(instruction & 0xFFFFu);

        if (word_count == 0 || index + word_count > spirv_words.size()) {
            throw std::runtime_error("Failed to parse SPIR-V: malformed instruction stream.");
        }
        if (opcode == OPCODE_ENTRY_POINT) {
            found_any_entry_point = true;
            const auto execution_model = spirv_words[index + 1];
            const auto name = read_spirv_string(&spirv_words[index + 2], word_count - 2);
            const auto stage = execution_model_to_shader_stage(execution_model);

            if (fallback_stage == std::nullopt) {
                fallback_stage = stage;
            }
        }
        index += word_count;
    }

    if (found_any_entry_point && fallback_stage.has_value()) {
        return *fallback_stage;
    }

    throw std::runtime_error("Failed to parse SPIR-V: no entry point found.");
}

} // namespace inexor::vulkan_renderer::wrapper::shaders
