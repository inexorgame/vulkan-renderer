#pragma once

#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <glm/vec3.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

// forward declaration
namespace inexor::vulkan_renderer::world {
class Cube;
} // namespace inexor::vulkan_renderer::world

// forward declaration
namespace inexor::vulkan_renderer::io {
class ByteStream;
template <std::size_t version>
[[nodiscard]] std::shared_ptr<world::Cube> deserialize_octree_impl(const ByteStream &stream);

} // namespace inexor::vulkan_renderer::io

void swap(inexor::vulkan_renderer::world::Cube &lhs, inexor::vulkan_renderer::world::Cube &rhs) noexcept;

namespace inexor::vulkan_renderer::world {

/// std::vector<Polygon> can probably replaced with an array.
using Polygon = std::array<glm::vec3, 3>;

using PolygonCache = std::shared_ptr<std::vector<Polygon>>;

class Cube : public std::enable_shared_from_this<Cube> {
    friend void ::swap(Cube &lhs, Cube &rhs) noexcept;
    template <std::size_t version>
    friend std::shared_ptr<world::Cube> io::deserialize_octree_impl(const io::ByteStream &stream);

public:
    /// Maximum of sub cubes (childs)
    static constexpr std::size_t SUB_CUBES = 8;
    /// Cube edges.
    static constexpr std::size_t EDGES = 12;
    /// Cube Type.
    enum class Type { EMPTY = 0b00U, SOLID = 0b01U, NORMAL = 0b10U, OCTANT = 0b11U };

private:
    Type m_type = Type::SOLID;
    float m_size = 32;
    glm::vec3 m_position{0.0F, 0.0F, 0.0F};

    /// Root cube points to itself.
    std::weak_ptr<Cube> m_parent = this->weak_from_this();

    /// Indentations, should only be used if it is a geometry cube.
    std::array<Indentation, Cube::EDGES> m_indentations{};
    std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> m_childs{};

    /// Only geometry cube (Type::SOLID and Type::Normal) have a polygon cache.
    mutable PolygonCache m_polygon_cache = nullptr;
    mutable bool m_polygon_cache_valid = false;

    /// Removes all childs recursive.
    void remove_childs();

    /// Get the root to this cube.
    [[nodiscard]] std::weak_ptr<Cube> root() const noexcept;
    /// Get the vertices of this cube. Use only on geometry cubes.
    [[nodiscard]] std::array<glm::vec3, 8> vertices() const noexcept;

public:
    Cube() = default;
    explicit Cube(Type type);
    Cube(Type type, float size, const glm::vec3 &position);
    Cube(std::weak_ptr<Cube> parent, Type type, float size, const glm::vec3 &position);
    Cube(const Cube &rhs);
    Cube(Cube &&rhs) noexcept;
    ~Cube() = default;
    Cube &operator=(Cube rhs);
    /// Get child.
    std::shared_ptr<Cube> operator[](std::size_t idx);
    /// Get child.
    const std::shared_ptr<const Cube> operator[](std::size_t idx) const;

    /// Is the current cube root.
    [[nodiscard]] bool is_root() const noexcept;
    /// At which child level this cube is.
    /// root cube = 0
    [[nodiscard]] std::size_t grid_level() const noexcept;
    /// Counts the number of Type::SOLID and Type::NORMAL cubes.
    [[nodiscard]] std::size_t count_geometry_cubes() const noexcept;

    /// Set a new type.
    void set_type(Type new_type);
    /// Get type.
    [[nodiscard]] Type type() const noexcept;

    /// Get childs.
    [[nodiscard]] const std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> &childs() const;
    /// Get indentations.
    [[nodiscard]] std::array<Indentation, Cube::EDGES> indentations() const noexcept;

    /// Set an indent by the edge id.
    void set_indent(std::uint8_t edge_id, Indentation indentation);
    /// Indent a specific edge by steps.
    /// @param positive_direction Indent in  positive axis direction.
    void indent(std::uint8_t edge_id, bool positive_direction, std::uint8_t steps);

    /// TODO: in special cases some polygons have no surface, if completely surrounded by others
    /// \warning Will update the cache even if it is considered as valid.
    void update_polygon_cache() const;
    /// Invalidate polygon cache.
    void invalidate_polygon_cache() const;
    /// Recursive way to collect all the caches.
    /// @param update_invalid If true it will update invalid polygon caches.
    [[nodiscard]] std::vector<PolygonCache> polygons(bool update_invalid = false) const;
};

} // namespace inexor::vulkan_renderer::world
