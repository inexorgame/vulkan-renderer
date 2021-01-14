#pragma once

#include <memory>
#include <utility>

// forward declaration
namespace inexor::vulkan_renderer::world {
class Cube;
} // namespace inexor::vulkan_renderer::world

// forward declaration
namespace inexor::vulkan_renderer::io {
class ByteStream;
} // namespace inexor::vulkan_renderer::io

namespace inexor::vulkan_renderer::io {

class OctreeParser {
public:
    /// Serialization of an octree.
    [[nodiscard]] virtual ByteStream serialize(std::shared_ptr<const world::Cube> cube, std::uint32_t version) = 0;
    /// Deserialization of an octree.
    [[nodiscard]] virtual std::shared_ptr<world::Cube> deserialize(const ByteStream &stream) = 0;
};

} // namespace inexor::vulkan_renderer::io
