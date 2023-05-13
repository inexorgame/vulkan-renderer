#pragma once

#include "inexor/vulkan-renderer/io/octree_parser.hpp"

#include <memory>

// Forward declaration
namespace inexor::vulkan_renderer::world {
class Cube;
} // namespace inexor::vulkan_renderer::world

// Forward declaration
namespace inexor::vulkan_renderer::io {
class ByteStream;
} // namespace inexor::vulkan_renderer::io

namespace inexor::vulkan_renderer::io {

class NXOCParser : public OctreeParser {
private:
    static constexpr std::uint32_t LATEST_VERSION{0};

    /// Specific version serialization.
    template <std::size_t version>
    [[nodiscard]] ByteStream serialize_impl(std::shared_ptr<const world::Cube> cube);
    /// Specific version deserialization.
    template <std::size_t version>
    [[nodiscard]] std::shared_ptr<world::Cube> deserialize_impl(const ByteStream &stream);

public:
    /// Serialization of an octree.
    [[nodiscard]] ByteStream serialize(std::shared_ptr<const world::Cube> cube, std::uint32_t version) final;
    /// Deserialization of an octree.
    [[nodiscard]] std::shared_ptr<world::Cube> deserialize(const ByteStream &stream) final;
};
} // namespace inexor::vulkan_renderer::io
