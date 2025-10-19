#include <gtest/gtest.h>

#include <inexor/vulkan-renderer/octree/collision_query.hpp>
#include <inexor/vulkan-renderer/octree/cube.hpp>

namespace inexor::vulkan_renderer {

TEST(CubeCollision, CollisionCheck) {
    const glm::vec3 world_pos{0, 0, 0};
    octree::Cube world(1.0f, world_pos);
    world.set_type(octree::Cube::Type::SOLID);

    glm::vec3 cam_pos{0.0f, 0.0f, 10.0f};
    glm::vec3 cam_direction{0.0f, 0.0f, 0.0f};

    auto collision1 = ray_cube_collision_check(world, cam_pos, cam_direction);
    bool collision_found = collision1.has_value();

    // There must be no collision for this data setup.
    EXPECT_FALSE(collision_found);

    cam_direction = {0.0f, 0.0f, -1.0f};

    auto collision2 = ray_cube_collision_check(world, cam_pos, cam_direction);
    collision_found = collision2.has_value();

    // Since we are now directly looking down on the cube, we collide with it.
    EXPECT_TRUE(collision_found);
}

} // namespace inexor::vulkan_renderer
