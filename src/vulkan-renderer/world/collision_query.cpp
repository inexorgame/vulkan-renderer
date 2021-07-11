#include "inexor/vulkan-renderer/world/collision_query.hpp"

#include <inexor/vulkan-renderer/world/cube.hpp>

#include <array>
#include <glm/gtx/intersect.hpp>

namespace inexor::vulkan_renderer::world {

bool ray_box_collision(const std::array<glm::vec3, 2> &box_bounds, const glm::vec3 &position,
                       const glm::vec3 &direction) {
    glm::vec3 inverse_dir{1 / direction.x, 1 / direction.y, 1 / direction.z};
    std::array<std::int32_t, 3> sign{static_cast<std::int32_t>(inverse_dir.x < 0),
                                     static_cast<std::int32_t>(inverse_dir.y < 0),
                                     static_cast<std::int32_t>(inverse_dir.z < 0)};

    float tmin{(box_bounds[sign[0]].x - position.x) * inverse_dir.x};
    float tmax{(box_bounds[1 - sign[0]].x - position.x) * inverse_dir.x};
    float tymin{(box_bounds[sign[1]].y - position.y) * inverse_dir.y};
    float tymax{(box_bounds[1 - sign[1]].y - position.y) * inverse_dir.y};

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }

    float tzmin{(box_bounds[sign[2]].z - position.z) * inverse_dir.z};
    float tzmax{(box_bounds[1 - sign[2]].z - position.z) * inverse_dir.z};

    return !((tmin > tzmax) || (tzmin > tmax));
}

std::optional<RayCubeCollision<Cube>> ray_cube_collision_check(const Cube &cube, const glm::vec3 pos,
                                                               const glm::vec3 dir,
                                                               const std::optional<std::uint32_t> grid_level_counter) {
    // If the cube is empty, a collision with a ray is not possible,
    // and there are no sub cubes to check for collision either.
    if (cube.type() == Cube::Type::EMPTY) {
        return std::nullopt;
    }

    // We need to pass this into glm::intersectRaySphere by reference although we are not interested in it.
    auto intersection_distance{0.0f};
    const auto bounding_sphere_radius = static_cast<float>(glm::sqrt(3) * cube.size()) / 2.0f;
    const auto sphere_radius_squared = static_cast<float>(std::pow(bounding_sphere_radius, 2));

    // First, check if the ray collides with the cube's bounding sphere.
    // Ray-sphere collision is much easier to calculate than ray-box collision.
    if (!glm::intersectRaySphere(pos, dir, cube.center(), sphere_radius_squared, intersection_distance)) {
        return std::nullopt;
    }

    // Second, check if ray collides with bounding box.
    // TODO: This is an axis aligned bounding box! Alignment must account for rotation of the octree in the future!
    if (!ray_box_collision(cube.bounding_box(), pos, dir)) {
        return std::nullopt;
    }

    if (cube.type() != Cube::Type::SOLID) {
        std::size_t hit_candidate_count{0};
        std::size_t collision_subcube_index{0};
        float m_nearest_square_distance = std::numeric_limits<float>::max();

        if (grid_level_counter.has_value()) {
            // Check if the maximum depth is reached.
            if (grid_level_counter.value() == 0) {
                // The maximum depth is reached but the cube is empty, no collision was found.
                if (cube.type() == Cube::Type::EMPTY) {
                    return std::nullopt;
                }

                // The current cube is of type OCTANT but not of type SOLID, but since we reached the maximum depth of
                // iteration, we treat it as type SOLID.
                // TODO: How can we solve respecting octree indentation for this case?
                return std::make_optional<RayCubeCollision<Cube>>(cube, pos, dir);
            }
        }

        auto subcubes = cube.children();

        // Iterate through all sub cubes and check for collision of the subcubes with the ray.
        for (std::int32_t i = 0; i < 8; i++) { // TODO: Turn into range loop?
            if (subcubes[i]->type() != Cube::Type::EMPTY) {
                const std::optional<std::uint32_t> next_grid_level =
                    grid_level_counter.has_value() ? std::make_optional<std::uint32_t>(grid_level_counter.value() + 1)
                                                   : std::nullopt;

                // No value for maximum depth given: Continue iterating until you find a leaf node of type SOLID.
                if (ray_cube_collision_check(*subcubes[i], pos, dir, next_grid_level)) {
                    hit_candidate_count++;

                    // If a ray collides with an octant, it can collide with multiple child cubes as it goes through it.
                    // We need to find the cube which is nearest to the camera and also in front of the camera.
                    const auto squared_distance = glm::distance2(subcubes[i]->center(), pos);

                    if (squared_distance < m_nearest_square_distance) {
                        collision_subcube_index = i;
                        m_nearest_square_distance = squared_distance;
                    }
                }
            }

            // If a ray goes through a cube of 8 subcubes, no more than 4 collisions can take place.
            // This condition holds true even when taking into account octree indentations.
            if (hit_candidate_count == 4) {
                break;
            }
        }

        if (hit_candidate_count > 0) {
            //
            return std::make_optional<RayCubeCollision<Cube>>(*subcubes[collision_subcube_index], pos, dir);
        }
    } else if (cube.type() == Cube::Type::SOLID) {
        //
        return std::make_optional<RayCubeCollision<Cube>>(cube, pos, dir);
    }

    return std::nullopt;
}

} // namespace inexor::vulkan_renderer::world
