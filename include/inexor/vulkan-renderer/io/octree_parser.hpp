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

constexpr std::uint32_t LATEST_OCTREE_FORMAT = 0;

/// Serialization of an octree.
[[nodiscard]] ByteStream serialize_octree(std::shared_ptr<const world::Cube> cube,
                                          std::uint32_t version = LATEST_OCTREE_FORMAT);
[[nodiscard]] std::shared_ptr<world::Cube> deserialize_octree(const ByteStream &stream);

/// Specific version serialization.
template <std::size_t version>
[[nodiscard]] ByteStream serialize_octree_impl(std::shared_ptr<const world::Cube> cube);
/// Specific version deserialization.
template <std::size_t version>
[[nodiscard]] std::shared_ptr<world::Cube> deserialize_octree_impl(const ByteStream &stream);

} // namespace inexor::vulkan_renderer::io
