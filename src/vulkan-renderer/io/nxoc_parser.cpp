#include "inexor/vulkan-renderer/io/nxoc_parser.hpp"

#include "inexor/vulkan-renderer/io/byte_stream.hpp"
#include "inexor/vulkan-renderer/io/io_exception.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <fstream>
#include <functional>
#include <utility>

namespace inexor::vulkan_renderer::io {
template <>
ByteStream NXOCParser::serialize_impl<0>(const std::shared_ptr<const world::Cube> cube) { // NOLINT
    ByteStreamWriter writer;
    writer.write<std::string>("Inexor Octree");
    writer.write<std::uint32_t>(0);

    std::function<void(const std::shared_ptr<const world::Cube> &)> iter_func;
    // pre-order traversal
    iter_func = [&iter_func, &writer](const std::shared_ptr<const world::Cube> &cube) {
        writer.write(cube->type());
        if (cube->type() == world::Cube::Type::OCTANT) {
            for (const auto &child : cube->children()) {
                iter_func(child);
            }
            return;
        }
        if (cube->type() == world::Cube::Type::NORMAL) {
            writer.write(cube->indentations());
        }
    };

    iter_func(cube);
    return writer;
}

template <>
std::shared_ptr<world::Cube> NXOCParser::deserialize_impl<0>(const ByteStream &stream) {
    ByteStreamReader reader(stream);
    std::shared_ptr<world::Cube> root = std::make_shared<world::Cube>();

    // Skip identifier, which is already checked.
    reader.skip(13);
    // Skip version.
    reader.skip(4);

    std::function<void(std::shared_ptr<world::Cube> &)> iter_func;
    // pre-order traversal
    iter_func = [&iter_func, &reader](std::shared_ptr<world::Cube> &cube) {
        cube->set_type(reader.read<world::Cube::Type>());
        if (cube->type() == world::Cube::Type::OCTANT) {
            for (auto child : cube->children()) {
                iter_func(child);
            }
            return;
        }
        if (cube->type() == world::Cube::Type::NORMAL) {
            cube->m_indentations = reader.read<std::array<world::Indentation, world::Cube::EDGES>>();
        }
    };
    iter_func(root);
    return root;
}

ByteStream NXOCParser::serialize(const std::shared_ptr<const world::Cube> cube, const std::uint32_t version) {
    if (cube == nullptr) {
        throw std::invalid_argument("cube cannot be a nullptr");
    }
    switch (version) { // NOLINT
    case 0:
        return serialize_impl<0>(cube);
    default:
        throw IoException("Unsupported octree version");
    }
}

std::shared_ptr<world::Cube> NXOCParser::deserialize(const ByteStream &stream) {
    ByteStreamReader reader(stream);
    if (reader.read<std::string>(13ull) != "Inexor Octree") {
        throw IoException("Wrong identifier");
    }
    const auto version = reader.read<std::uint32_t>();
    switch (version) { // NOLINT
    case 0:
        return deserialize_impl<0>(stream);
    default:
        throw IoException("Unsupported octree version");
    }
}
} // namespace inexor::vulkan_renderer::io
