#pragma once

#include "inexor/vulkan-renderer/world/collision.hpp"

#include <glm/vec3.hpp>

#include <optional>

// Forward declaration
namespace inexor::vulkan_renderer::world {
class Cube;
} // namespace inexor::vulkan_renderer::world

namespace inexor::vulkan_renderer::world {

// TODO: Implement PointCubeCollision

/// @brief ``True`` of the ray build from the two vectors collides with the cube's bounding box.
/// @note There is no such function as glm::intersectRayBox
/// @param box_bounds An array of two vectors which represent the edges of the bounding box
/// @param pos The start position of the ray
/// @param dir The direction of the ray
/// @return ``True`` if the ray collides with the octree cube's bounding box
[[nodiscard]] bool ray_box_collision(std::array<glm::vec3, 2> &box_bounds, glm::vec3 &pos, glm::vec3 &dir);

/// @brief Check for a collision between a camera ray and octree geometry.
/// @param cube The cube to check collisions with
/// @param pos The camera position
/// @param dir The camera view direction
/// @param max_depth The maximum subcube iteration depth. If this depth is reached and the cube is an octant, it
/// will be treated as if it was a solid cube. This is the foundation for the implementation of grid size in octree
/// editor
/// @note This does not account yet for octree indentation
/// @return A std::optional which contains the collision data (if any collision was found)
[[nodiscard]] std::optional<RayCubeCollision<Cube>>
ray_cube_collision_check(const Cube &cube, glm::vec3 pos, glm::vec3 dir,
                         std::optional<std::uint32_t> grid_level_counter = std::nullopt);

} // namespace inexor::vulkan_renderer::world
