#include "inexor/vulkan-renderer/io/octree_parser.hpp"
#include "inexor/vulkan-renderer/io/byte_stream.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <fstream>
#include <functional>
#include <utility>

namespace inexor::vulkan_renderer::io {
template <>
ByteStream serialize_octree_impl<0>(const std::shared_ptr<const world::Cube> cube) {
    if (cube == nullptr) {
        throw std::runtime_error("cube cannot be a nullptr.");
    }
    ByteStreamWriter writer;
    writer.write<std::string>("Inexor Octree");
    writer.write<std::uint32_t>(0);

    std::function<void(const std::shared_ptr<const world::Cube> &)> iter_func;
    // pre-order traversal
    iter_func = [&iter_func, &writer](const std::shared_ptr<const world::Cube> &cube) {
        writer.write(cube->type());
        if (cube->type() == world::Cube::Type::OCTANT) {
            for (const auto &child : cube->childs()) {
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
};

template <>
std::shared_ptr<world::Cube> deserialize_octree_impl<0>(const ByteStream &stream) {
    ByteStreamReader reader(stream);
    if (reader.read<std::string>(std::size_t(13)) != "Inexor Octree") {
        throw std::runtime_error("Wrong identifier.");
    }
    if (reader.read<std::uint32_t>() != 0) {
        throw std::runtime_error("Mismatched version.");
    }
    std::shared_ptr<world::Cube> root = std::make_shared<world::Cube>();

    std::function<void(std::shared_ptr<world::Cube> &)> iter_func;
    // pre-order traversal
    iter_func = [&iter_func, &reader](std::shared_ptr<world::Cube> &cube) {
        cube->set_type(reader.read<world::Cube::Type>());
        if (cube->type() == world::Cube::Type::OCTANT) {
            for (auto child : cube->childs()) {
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

ByteStream serialize_octree(const std::shared_ptr<const world::Cube> cube, const std::uint32_t version) {
    switch (version) {
    case 0:
        return serialize_octree_impl<0>(cube);
    default:
        throw std::runtime_error("Unsupported octree version.");
    };
}

std::shared_ptr<world::Cube> deserialize_octree(const ByteStream &stream) {
    ByteStreamReader reader(stream);
    if (reader.read<std::string>(std::size_t(13)) != "Inexor Octree") {
        throw std::runtime_error("Wrong identifier.");
    }
    const std::uint32_t version = reader.read<std::uint32_t>();
    switch (version) {
    case 0:
        return deserialize_octree_impl<0>(stream);
    default:
        throw std::runtime_error("Unsupported octree version.");
    };
}
} // namespace inexor::vulkan_renderer::io
