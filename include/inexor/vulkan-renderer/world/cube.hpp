#pragma once

#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec3.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

// forward declaration
namespace inexor::vulkan_renderer::world {
class Cube;
} // namespace inexor::vulkan_renderer::world

// forward declaration
namespace inexor::vulkan_renderer::io {
class ByteStream;
class NXOCParser;
} // namespace inexor::vulkan_renderer::io

void swap(inexor::vulkan_renderer::world::Cube &lhs, inexor::vulkan_renderer::world::Cube &rhs) noexcept;

namespace inexor::vulkan_renderer::world {

/// std::vector<Polygon> can probably replaced with an array.
using Polygon = std::array<glm::vec3, 3>;

using PolygonCache = std::shared_ptr<std::vector<Polygon>>;

class Cube : public std::enable_shared_from_this<Cube> {
    friend void ::swap(Cube &lhs, Cube &rhs) noexcept;
    friend class io::NXOCParser;

public:
    /// Maximum of sub cubes (children)
    static constexpr std::size_t SUB_CUBES{8};
    /// Cube edges.
    static constexpr std::size_t EDGES{12};
    /// Cube Type.
    enum class Type { EMPTY = 0b00u, SOLID = 0b01u, NORMAL = 0b10u, OCTANT = 0b11u };
    enum class NeighborAxis { X = 2, Y = 1, Z = 0 };
    enum class NeighborDirection { POSITIVE, NEGATIVE };

    /// IDs of the children and edges which will be swapped to receive the rotation.
    /// To achieve a 90 degree rotation the 0th index have to be swapped with the 1st and the 1st with the 2nd, etc.
    struct RotationAxis {
        using ChildType = std::array<std::array<std::size_t, 4>, 2>;
        using EdgeType = std::array<std::array<std::size_t, 4>, 3>;
        using Type = std::pair<ChildType, EdgeType>;
        /// IDs of the children / edges which will be swapped to receive the rotation around X axis.
        static constexpr Type X{{{{0, 1, 3, 2}, {4, 5, 7, 6}}}, {{{2, 4, 11, 1}, {5, 7, 8, 10}, {0, 9, 6, 3}}}};
        /// IDs of the children / edges which will be swapped to receive the rotation around Y axis.
        static constexpr Type Y{{{{0, 4, 5, 1}, {2, 6, 7, 3}}}, {{{0, 5, 9, 2}, {3, 8, 6, 11}, {1, 10, 7, 4}}}};
        /// IDs of the children / edges which will be swapped to receive the rotation around Z axis.
        static constexpr Type Z{{{{0, 2, 6, 4}, {1, 3, 7, 5}}}, {{{1, 3, 10, 0}, {4, 6, 7, 9}, {2, 11, 8, 5}}}};
    };

private:
    Type m_type{Type::EMPTY};
    float m_size{32};
    glm::vec3 m_position{0.0f, 0.0f, 0.0f};

    /// Root cube is empty.
    std::weak_ptr<Cube> m_parent{};

    /// Index of this in m_parent.m_children; undefined behavior if root.
    std::uint8_t m_index_in_parent{};

    /// Indentations, should only be used if it is a geometry cube.
    std::array<Indentation, Cube::EDGES> m_indentations;
    std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> m_children;

    /// Only geometry cube (Type::SOLID and Type::Normal) have a polygon cache.
    mutable PolygonCache m_polygon_cache;
    mutable bool m_polygon_cache_valid{false};

    /// Removes all children recursive.
    void remove_children();

    /// Get the root to this cube.
    [[nodiscard]] std::shared_ptr<Cube> root() noexcept;
    /// Get the vertices of this cube. Use only on geometry cubes.
    [[nodiscard]] std::array<glm::vec3, 8> vertices() const noexcept;

    /// Optimized implementations of 90°, 180° and 270° rotations.
    template <int Rotations>
    void rotate(const RotationAxis::Type &axis);

public:
    /// Create an empty cube.
    Cube() = default;
    /// Create an empty cube.
    Cube(float size, const glm::vec3 &position);
    /// Create an empty cube.
    Cube(std::weak_ptr<Cube> parent, std::uint8_t index, float size, const glm::vec3 &position);
    /// Use clone() to create an independent copy of a cube.
    Cube(const Cube &rhs) = delete;
    Cube(Cube &&rhs) noexcept;
    ~Cube() = default;
    Cube &operator=(Cube rhs);
    Cube &operator=(Cube &&) = delete;

    /// Get child.
    std::shared_ptr<Cube> operator[](std::size_t idx);
    /// Get child.
    std::shared_ptr<const Cube> operator[](std::size_t idx) const; // NOLINT

    /// Clone a cube, which has no relations to the current one or its children.
    /// It will be a root cube.
    [[nodiscard]] std::shared_ptr<Cube> clone() const;

    /// Is the current cube root.
    [[nodiscard]] bool is_root() const noexcept;
    /// At which child level this cube is.
    /// root cube = 0
    [[nodiscard]] std::size_t grid_level() const noexcept;
    /// Count the number of Type::SOLID and Type::NORMAL cubes.
    [[nodiscard]] std::size_t count_geometry_cubes() const noexcept;

    [[nodiscard]] glm::vec3 center() const noexcept {
        return m_position + 0.5f * m_size;
    }

    [[nodiscard]] glm::vec3 position() const noexcept {
        return m_position;
    }

    [[nodiscard]] std::array<glm::vec3, 2> bounding_box() const {
        return {m_position, {m_position.x + m_size, m_position.y + m_size, m_position.z + m_size}};
    }

    [[nodiscard]] float size() const noexcept {
        return m_size;
    }

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

    /// Rotate the cube 90° clockwise around the given axis. Repeats with the given rotations.
    /// @param axis Only one index should be one.
    /// @param rotations Value does not need to be adjusted beforehand. (e.g. mod 4)
    void rotate(const RotationAxis::Type &axis, int rotations);

    /// TODO: in special cases some polygons have no surface, if completely surrounded by others
    /// \warning Will update the cache even if it is considered as valid.
    void update_polygon_cache() const;
    /// Invalidate polygon cache.
    void invalidate_polygon_cache() const;
    /// Recursive way to collect all the caches.
    /// @param update_invalid If true it will update invalid polygon caches.
    [[nodiscard]] std::vector<PolygonCache> polygons(bool update_invalid = false) const;

    /// Get the (face) neighbor of this cube by using a similar implementation to Samets "OT_GTEQ_FACE_NEIGHBOR(P,I)".
    /// @brief Get the (face) neighbor of this cube.
    /// @param axis The axis on which to get the neighboring cube
    /// @param direction Whether to get the cube which is above or below this cube on the selected axis
    /// @returns Same-sized neighbor if existent, else larger neighbor if exists, otherwise (i.e. when no neighbor
    /// exists) nullptr.
    /// @see Samet, H. (1989) [Neighbor finding in Images Represented by Octrees.]
    /// (https://web.archive.org/web/20190712063957/http://www.cs.umd.edu/~hjs/pubs/SameCVGIP89.pdf)
    /// Computer Vision, Graphics, and Image Processing. 46 (3), 367-386.
    [[nodiscard]] std::shared_ptr<Cube> neighbor(NeighborAxis axis, NeighborDirection direction);
};

std::shared_ptr<world::Cube> create_random_world(std::uint32_t max_depth, const glm::vec3 &position,
                                                 std::optional<std::uint32_t> = std::nullopt);

} // namespace inexor::vulkan_renderer::world
