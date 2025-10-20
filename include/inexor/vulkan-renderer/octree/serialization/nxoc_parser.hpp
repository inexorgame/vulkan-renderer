#pragma once

#include "inexor/vulkan-renderer/octree/serialization/octree_parser.hpp"

#include <memory>

// Forward declaration
namespace inexor::vulkan_renderer::octree {
class Cube;
} // namespace inexor::vulkan_renderer::octree

// Forward declaration
namespace inexor::vulkan_renderer::serialization {
class ByteStream;
} // namespace inexor::vulkan_renderer::serialization

namespace inexor::vulkan_renderer::serialization {

class NXOCParser : public OctreeParser {
private:
    static constexpr std::uint32_t LATEST_VERSION{0};

    /// Specific version deserialization.
    template <std::size_t version>
    [[nodiscard]] std::shared_ptr<octree::Cube> deserialize_impl(const ByteStream &stream);

    /// Specific version serialization.
    template <std::size_t version>
    [[nodiscard]] ByteStream serialize_impl(std::shared_ptr<const octree::Cube> cube);

public:
    /// Deserialization of an octree.
    [[nodiscard]] std::shared_ptr<octree::Cube> deserialize(const ByteStream &stream) final;

    /// Serialization of an octree.
    [[nodiscard]] ByteStream serialize(std::shared_ptr<const octree::Cube> cube, std::uint32_t version) final;
};
} // namespace inexor::vulkan_renderer::serialization
