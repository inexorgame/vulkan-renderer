#pragma once

#include "inexor/vulkan-renderer/world/bit_stream.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/signals2.hpp>
#include <glm/vec3.hpp>

#include <functional>
#include <iostream>
#include <optional>

namespace inexor::vulkan_renderer::world {
/// How often a cube can be indented, results in MAX_INDENTATION+1 steps.
constexpr std::uint8_t MAX_INDENTATION = 8;
/// The default size of a cube / the octree size boundaries.
constexpr float DEFAULT_CUBE_SIZE = 1;
/// The default position of the cube in the coordinate system.
constexpr glm::vec3 DEFAULT_CUBE_POSITION = {0., 0., 0.};

/// The types a cube can have and its bit-representation.
enum class CubeType {
    /// The cube has no surface and no vertices.
    EMPTY = 0b00,
    /// The cube is a "real" cube where each edge has the same length.
    FULL = 0b01,
    /// The cube has at least on edge which has been indented from at least one axis.
    /// That means that the cube may or may not be a perfect cube with same-length edges.
    INDENTED = 0b10,
    /// The cube is divided in 8 octants (same-sized cubes).
    OCTANT = 0b11
};

class Indentation {
private:
    /// Run on-change events.
    void change();

    /// Copy the values from another indentation to this one.
    /// @param indentation The indentation to copy the values from.
    /// @return Whether any value has changed.
    bool copy_values(const Indentation &indentation);

    /// Parse one axis from a BitStream.
    /// @param stream The BitStream to parse one axis from.
    /// @return The indentation level on that axis.
    static std::uint8_t parse_one(BitStream &stream);

    /// Indentation level on the x-axis.
    std::uint8_t x_level = 0;

    /// Indentation level on the y-axis.
    std::uint8_t y_level = 0;

    /// Indentation level on the z-axis.
    std::uint8_t z_level = 0;

public:
    /// Create an Indentation to assign to a cube corner.
    Indentation();

    /// Create an Indentation to assign to a cube corner.
    /// @param x Indentation leven on the x-axis
    /// @param y Indentation leven on the y-axis
    /// @param z Indentation leven on the z-axis
    Indentation(std::uint8_t x, std::uint8_t y, std::uint8_t z);

    Indentation(const Indentation& indentation);
    Indentation(Indentation &&indentation) noexcept;

    /// Signal emitted when one of the indentation levels changes.
    /// Argument: the indentation emitting the signal ("this").
    boost::signals2::signal<void(Indentation *)> on_change;

    Indentation& operator=(Indentation&& lhs) noexcept;
    Indentation& operator=(const Indentation& rhs);
    Indentation& operator=(const glm::tvec3<uint8_t>& rhs);
    Indentation& operator+=(const glm::tvec3<int8_t>& other);
    Indentation& operator-=(const glm::tvec3<int8_t>& other);

    [[nodiscard]] bool equal_values(const Indentation &other) const;
    [[nodiscard]] bool equal_values(const glm::tvec3<uint8_t> &other) const;

    /// Set the indentations depth for the axis.
    /// @param x Indentation leven on the x-axis.
    /// @param y Indentation leven on the y-axis.
    /// @param z Indentation leven on the z-axis.
    void set(std::optional<std::uint8_t> x, std::optional<std::uint8_t> y, std::optional<std::uint8_t> z);

    /// Set the indentation level for the x axis.
    /// @param x Indentation level on the x axis.
    void set_x(std::uint8_t x);

    /// Set the indentation level for the y axis.
    /// @param x Indentation level on the y axis.
    void set_y(std::uint8_t y);

    /// Set the indentation level for the z axis.
    /// @param x Indentation level on the z axis.
    void set_z(std::uint8_t z);

    /// Parse an indentation from a bitstream.
    /// @param stream The stream to extract the bitstream from.
    /// @return The indentation on all three axes.
    static Indentation parse(BitStream &stream);

    /// Get the x-axis indentation level.
    /// @return the x-axis indentation level.
    [[nodiscard]] std::uint8_t x() const;

    /// Get the y-axis indentation level.
    /// @return the y-axis indentation level.
    [[nodiscard]] std::uint8_t y() const;

    /// Get the z-axis indentation level.
    /// @return the z-axis indentation level.
    [[nodiscard]] std::uint8_t z() const;

    /// Get the indentation levels on all three axes as a glm::tvec3<std::uint8_t>.
    /// @return indentation levels on all three axes
    [[nodiscard]] glm::tvec3<std::uint8_t> vec() const;

    // TODO: Get bits from values.
    // [[nodiscard]] dynamic_bitset<> bits();
};

/// A cube or octree representing the maps geometry.
///
/// Values connected to corners of cubes are saved in the following order.
///
/// | Order | X      | Y      | Z      |
/// |-------|--------|--------|--------|
/// | 1.    | lower  | lower  | lower  |
/// | 2.    | lower  | lower  | higher |
/// | 3.    | lower  | higher | lower  |
/// | 4.    | lower  | higher | higher |
/// | 5.    | higher | lower  | lower  |
/// | 6.    | higher | lower  | higher |
/// | 7.    | higher | higher | lower  |
/// | 8.    | higher | higher | higher |
///
/// Values connected to sides of cubes are saved in the following order.
///
/// | Order | X      | Y      | Z      |
/// |-------|--------|--------|--------|
/// | 1.    | axis   | axis   | lower  |
/// | 2.    | axis   | axis   | higher |
/// | 3.    | axis   | lower  | axis   |
/// | 4.    | axis   | higher | axis   |
/// | 5.    | lower  | axis   | axis   |
/// | 6.    | higher | axis   | axis   |
class Cube {
private:
    Cube(
        CubeType type,
        float size,
        const glm::vec3 &position,
        const std::optional<std::array<Indentation, 8>> &indentations,
        std::optional<std::array<std::shared_ptr<Cube>, 8>> octants
        );

    /// Insert all polygons into memory.
    /// @param polygons Pointer to the memory where the polygons should be saved to.
    void all_polygons(std::array<glm::vec3, 3> *&polygons);

    /// Run on-change events.
    void change();

    /// Run on-change events.
    void change(Indentation *indentation);

    /// Copy the values from another cube to this one.
    /// @param cube The cube to copy the values from.
    /// @return Whether any value has changed (this != &cube).
    bool copy_values(const Cube &cube);

    /// Get all vertices of this cube (not its children).
    /// @return vertices of this cube.
    [[nodiscard]] std::array<glm::vec3, 8> vertices();

    /// Get the polygons of this cube as if it is a full cube.
    /// @return polygons of this cube as if it is a full cube
    std::array<std::array<glm::vec3, 3>, 12> full_polygons();

    /// Get the vertices in a structure which is ordered in triangles of the order of a full cube.
    /// @param v The vertices of the the sides of a cube.
    /// @return polygons of this cube in the order of a full cube.
    std::array<std::array<glm::vec3, 3>, 12> full_polygons(std::array<glm::vec3, 8> &v);

    /// Get the polygons of this cube (only when it is an indented cube).
    /// @return polygons of this cube
    std::array<std::array<glm::vec3, 3>, 12> indented_polygons();

    /// Get the indentation levels for each side of the cube.
    /// @return The indentation lebvels for each side of the cube.
    std::array<glm::tvec3<std::uint8_t>, 8> indentation_levels();

    /// Cache of this cubes polygons. Not of its octants (i.e., empty of the cube is of type CubeType::OCTANTS).
    std::array<std::array<glm::vec3, 3>, 12> polygons_cache = {}; // Vertices of this cube (not its octants)

    /// Whether this->polygons_cache is valid and might be used.
    bool valid_cache = false;

    /// Whether this octree is reactive (i.e. updates when childs update).
    bool is_reactive = false;

    /// Type of the cube.
    CubeType cube_type = CubeType::EMPTY;

    /// The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
    /// x, y, and z-axis).
    glm::vec3 cube_position = {0.0f, 0.0f, 0.0f};

    /// The maximum size of the cube (i.e. if the cube is not indented).
    float cube_size = 32;

public:
    /// Signal emitted when any of the geometry of this cube or its child cubes changes.
    /// Argument: the cube which was originally changed ("this" or a child-cube).
    boost::signals2::signal<void(Cube *)> on_change;

    /// The indentations of this cube if this cube is of CubeType::INDENTED.
    /// Ordered as following:
    /// 0. Corner with lower x-axis-value, lower y-value, lower z-value.
    /// 1. Corner with lower x-axis-value, lower y-value, higher z-value.
    /// 2. Corner with lower x-axis-value, higher y-value, lower z-value.
    /// 3. Corner with lower x-axis-value, higher y-value, higher z-value.
    /// 4. Corner with higher x-axis-value, lower y-value, lower z-value.
    /// 5. Corner with higher x-axis-value, lower y-value, higher z-value.
    /// 6. Corner with higher x-axis-value, higher y-value, lower z-value.
    /// 7. Corner with higher x-axis-value, higher y-value, higher z-value.
    std::optional<std::array<Indentation, 8>> indentations = std::nullopt;

    /// The octants of this cube if this cube is of CubeType::OCTANTS.
    /// Ordered as following:
    /// 0. Octant with lower x-axis-value, lower y-value, lower z-value.
    /// 1. Octant with lower x-axis-value, lower y-value, higher z-value.
    /// 2. Octant with lower x-axis-value, higher y-value, lower z-value.
    /// 3. Octant with lower x-axis-value, higher y-value, higher z-value.
    /// 4. Octant with higher x-axis-value, lower y-value, lower z-value.
    /// 5. Octant with higher x-axis-value, lower y-value, higher z-value.
    /// 6. Octant with higher x-axis-value, higher y-value, lower z-value.
    /// 7. Octant with higher x-axis-value, higher y-value, higher z-value.
    std::optional<std::array<std::shared_ptr<Cube>, 8>> octants = std::nullopt;

    /// Create a cube.
    /// @param type The type of the cube. The cube needs further adjustment after construction if not of CubeType::FULL or CubeType::EMPTY.
    /// @param size The maximum size of the cube.
    /// @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
    /// x, y, and z-axis).
    Cube(CubeType type, float size, const glm::vec3 &position);

    /// Create a CubeType::INDENTED cube.
    /// @param indentations The indentations of the cube.
    /// @param size The maximum size of the cube.
    /// @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
    /// x, y, and z-axis).
    Cube(std::array<Indentation, 8> &indentations, float size, const glm::vec3 &position);

    /// Create a CubeType::Octants cube.
    ///
    /// @param octants The octants (i.e. sub-cubes) of the cube.
    /// @param size The maximum size of the cube.
    /// @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
    /// x, y, and z-axis).
    Cube(std::array<std::shared_ptr<Cube>, 8> &octants, float size, const glm::vec3 &position);

    Cube(const Cube& cube);
    Cube(Cube &&cube) noexcept;

    Cube& operator=(Cube&& lhs) noexcept;
    Cube& operator=(const Cube& rhs);

    /// Parse an octree from binary data.
    /// @param data The data to parse the octree from.
    /// @return Cube object representing the cubes / octrees from the data.
    static Cube parse(std::vector<unsigned char> &data);

    /// Parse an octree from a BitStream.
    /// @param stream The BitStream to parse the octree from.
    /// @return Cube object representing the cubes / octrees from the stream.
    static Cube parse(BitStream &stream);

    /// Parse an octree from a BitStream.
    /// @param stream The BitStream to parse the octree from.
    /// @param size The maximum size of the cube.
    /// @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
    /// x, y, and z-axis).
    /// @return Cube object representing the cubes / octrees from the stream.
    static Cube parse(BitStream &stream, float size, const glm::vec3 &position);

    /// Get the type of the cube.
    /// @return type of the cube.
    [[nodiscard]] CubeType type();

    /// Get the number of leaves, this octree contains.
    /// Leaves are cubes of CubeType::INDENTED or CubeTYPE::FULL.
    /// @return Number of leaves, this octree contains.
    [[nodiscard]] std::uint64_t leaves();

    // [[nodiscard]] dynamic_bitset<> bits(); // Bit representation of this cube
    // [[nodiscard]] vector<array<glm::vec3, 8>> vertices(); // All vertices this cube contains.
    // [[nodiscard]] vector<array<glm::vec3, 4>> sides(); // All sides this cube contains.
    /// Get all polygons (triangles) of each cube of this octree.
    /// @return A vector which contains the three vertices representing a triangle.
    [[nodiscard]] std::vector<std::array<glm::vec3, 3>> polygons(); // All polygons this cube contains.

    /// Invalidate the cache of this cube / octree (not its children).
    void invalidate_cache();

    /// Make this octree reactive (update its values when one of its attributes changes).
    /// @param force Whether to make it reactive again even though the connections were established before.
    void make_reactive(bool force = false);
};
} // namespace inexor::vulkan_renderer::world
