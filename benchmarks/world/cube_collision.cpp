#include <benchmark/benchmark.h>

#include <inexor/vulkan-renderer/world/collision_query.hpp>
#include <inexor/vulkan-renderer/world/cube.hpp>

namespace inexor::vulkan_renderer {

void CubeCollision(benchmark::State &state) {
    for (auto _ : state) {
        const glm::vec3 world_pos{0, 0, 0};
        world::Cube world(1.0f, world_pos);
        world.set_type(world::Cube::Type::SOLID);

        glm::vec3 cam_pos{0.0f, 0.0f, 10.0f};
        glm::vec3 cam_direction{0.0f, 0.0f, -1.0f};

        benchmark::DoNotOptimize(ray_cube_collision_check(world, cam_pos, cam_direction));
    }
}

BENCHMARK(CubeCollision);

}; // namespace inexor::vulkan_renderer
