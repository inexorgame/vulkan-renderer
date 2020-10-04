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

/// IDs of the children in a order so that we rotate the octants when moving the child with the first ID to the second
/// ID, the second, to the third, ...
using ChildRotationOrder = std::array<std::array<uint8_t, 4>, 2>;

/// IDs of the edges in a order so that we rotate the cube when moving the edge with the first ID to the second ID,
/// the second, to the third, ...
/// As we need to update the indentations, it is required that first two indentations in the array
/// point in the correct order towards the same corner as the second two corners. Example:
///    -1->       Correct are either {{0, 1, 3, 2}} or {{3, 2, 0, 1}}
///   ^    ^      Incorrect are all other variants.
/// 0 |    | 2
///    -3->
using EdgeRotationOrder = std::array<std::array<uint8_t, 4>, 3>;

class Cube : public std::enable_shared_from_this<Cube> {
    friend void ::swap(Cube &lhs, Cube &rhs) noexcept;
    template <std::size_t version>
    friend std::shared_ptr<world::Cube> io::deserialize_octree_impl(const io::ByteStream &stream);

public:
    /// Maximum number of sub cubes (children, i.e. octants)
    static constexpr std::size_t SUB_CUBES = 8;
    /// Number of edges a cube has.
    static constexpr std::size_t EDGES = 12;

    /// IDs of the edges in the order of for a 90° rotation around the X-axis.
    static constexpr EdgeRotationOrder X_EDGE_ROTATION_ORDER{
        {// First edge need to point to (crossing with) second, second to third, third to second (again) and fourth to
         // third
         {{2, 4, 11, 1}},
         {{5, 7, 8, 10}},
         {{0, 9, 6, 3}}}};

    /// IDs of the edges in the order of for a 90° rotation around the Y-axis.
    static constexpr EdgeRotationOrder Y_EDGE_ROTATION_ORDER{{{{0, 5, 9, 2}}, {{3, 8, 6, 11}}, {{1, 10, 7, 4}}}};

    /// IDs of the edges in the order of for a 90° rotation around the Z-axis.
    static constexpr EdgeRotationOrder Z_EDGE_ROTATION_ORDER{{{{1, 3, 10, 0}}, {{4, 6, 7, 9}}, {{2, 11, 8, 5}}}};

    /// IDs of the children in the order of for a 90° rotation around the X-axis.
    static constexpr ChildRotationOrder X_CHILD_ROTATION_ORDER{{{{0, 1, 3, 2}}, {{4, 5, 7, 6}}}};

    /// IDs of the children in the order of for a 90° rotation around the Y-axis.
    static constexpr ChildRotationOrder Y_CHILD_ROTATION_ORDER{{{{0, 4, 5, 1}}, {{2, 6, 7, 3}}}};

    /// IDs of the children in the order of for a 90° rotation around the Z-axis.
    static constexpr ChildRotationOrder Z_CHILD_ROTATION_ORDER{{{{0, 2, 6, 4}}, {{1, 3, 7, 5}}}};

    /// Cube Type.
    enum class Type {
        /// The cube does not have any faces.
        EMPTY = 0b00U,
        /// The cube has six equal faces (no indentations).
        SOLID = 0b01U,
        /// The cube has at least one indentation.
        NORMAL = 0b10U,
        /// The cube is divided in 8 sub-cubes (children/octants).
        OCTANT = 0b11U
    };

private:
    Type m_type{Type::SOLID};
    float m_size{32};
    glm::vec3 m_position{0.0F, 0.0F, 0.0F};

    /// Root cube points to itself.
    std::weak_ptr<Cube> m_parent{weak_from_this()};

    /// Indentations, should only be used if it is a geometry cube.
    std::array<Indentation, Cube::EDGES> m_indentations{};
    std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> m_children{};

    /// Only geometry cube (Type::SOLID and Type::Normal) have a polygon cache.
    mutable PolygonCache m_polygon_cache{nullptr};
    mutable bool m_polygon_cache_valid{false};

    /// Removes all children recursively.
    void remove_children();

    /// Rotate a cube.
    /// The (child-, and edge-)orders implicitly contain the axis around which the cube will be rotated.
    /// @param rotation_level The degree to which a cube should be rotated:
    ///                       lvl mod 4 = 0 -> no rotation
    ///                       lvl mod 4 = 1 -> 90°
    ///                       lvl mod 4 = 2 -> 180°
    ///                       lvl mod 4 = 3 -> 270°
    /// @param edge_order The rotation-orders of the edges which should be rotated. At a 90° rotation, each first
    ///                   element will go to the place of the second, each second to the third, etc.
    /// @param child_order The rotation-orders of the children which should be rotated. At a 90° rotation, each first
    ///                    element will go to the place of the second, each second to the third, etc.
    void rotate(const uint8_t &rotation_level, const EdgeRotationOrder &edge_order,
                const ChildRotationOrder &child_order);

    /// Rotate a cube by 90°.
    /// @param edge_order The rotation-orders of the edges which should be rotated.
    /// @param child_order The rotation-orders of the children which should be rotated.
    void rotate_90(const EdgeRotationOrder &edge_order, const ChildRotationOrder &child_order);
    /// Rotate elements by 90° (must contain the correct order for the rotation).
    /// @param order The rotation-order of the elements which should be rotated.
    template <typename TYPE, std::size_t SIZE>
    static void rotate_elements_90(const std::array<uint8_t, 4> &order, std::array<TYPE, SIZE> &elements);

    /// Rotate a cube by 180°.
    /// @param edge_order The rotation-orders of the edges which should be rotated.
    /// @param child_order The rotation-orders of the children which should be rotated.
    void rotate_180(const EdgeRotationOrder &edge_order, const ChildRotationOrder &child_order);
    template <typename TYPE, std::size_t SIZE>
    /// Rotate elements by 180° (must contain the correct order for the rotation).
    /// @param order The rotation-order of the elements which should be rotated.
    static void rotate_elements_180(const std::array<uint8_t, 4> &order, std::array<TYPE, SIZE> &elements);

    /// Rotate a cube by 270°.
    /// @param edge_order The rotation-orders of the edges which should be rotated.
    /// @param child_order The rotation-orders of the children which should be rotated.
    void rotate_270(const EdgeRotationOrder &edge_order, const ChildRotationOrder &child_order);
    /// Rotate elements by 270° (must contain the correct order for the rotation).
    /// @param order The rotation-order of the elements which should be rotated.
    template <typename TYPE, std::size_t SIZE>
    static void rotate_elements_270(const std::array<uint8_t, 4> &order, std::array<TYPE, SIZE> &elements);

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

    /// Get children.
    [[nodiscard]] const std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> &children() const;
    /// Get indentations.
    [[nodiscard]] std::array<Indentation, Cube::EDGES> indentations() const noexcept;

    /// Set an indent by the edge id.
    void set_indent(std::uint8_t edge_id, Indentation indentation);
    /// Indent a specific edge by steps.
    /// @param positive_direction Indent in  positive axis direction.
    void indent(std::uint8_t edge_id, bool positive_direction, std::uint8_t steps);
    /// Rotate a cube on one axis.
    /// @param axis Axis to rotate the cube around. Only one of x, y, or z may not be 0. If this that value
    ///              - mod 4 = 0 then no rotation
    ///              - mod 4 = 1 then a 90° rotation
    ///              - mod 4 = 2 then a 180° rotation
    ///              - mod 4 = 3 then a 270° rotation
    ///             will be performed.
    void rotate(const glm::vec<3, int8_t> &axis);

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
