#pragma once

#include <cstdint>
#include <memory>

// Forward declaration
namespace inexor::vulkan_renderer::octree {
class Cube;
} // namespace inexor::vulkan_renderer::octree

namespace inexor::vulkan_renderer::serialization {

// Forward declaration
class ByteStream;

class OctreeParser {
public:
    /// Serialization of an octree.
    [[nodiscard]] virtual ByteStream serialize(std::shared_ptr<const octree::Cube> cube, std::uint32_t version) = 0;
    /// Deserialization of an octree.
    [[nodiscard]] virtual std::shared_ptr<octree::Cube> deserialize(const ByteStream &stream) = 0;
};

} // namespace inexor::vulkan_renderer::serialization
