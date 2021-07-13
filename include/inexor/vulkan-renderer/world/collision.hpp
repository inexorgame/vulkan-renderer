#pragma once

#include <glm/vec3.hpp>

#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace inexor::vulkan_renderer::world {

/// @brief A wrapper for collision data which describes ray-octree collision.
/// This class is used for octree collision, but it can be used for every cube-like data structure
/// @tparam T A type which offers a size() and center() method.
template <typename T>
class RayCubeCollision {
    const std::shared_ptr<T> m_cube;

    // If we find a ray-cube collision, there will always be a collision with the bounding box.
    // However a collision with the cube's vertex geometry is no always present.
    std::optional<glm::vec3> m_vertex_intersection;

    glm::vec3 m_cube_intersection;
    glm::vec3 m_cube_face;
    glm::vec3 m_nearest_cube_corner;
    glm::vec3 m_nearest_cube_edge;

public:
    RayCubeCollision(std::shared_ptr<T> cube, glm::vec3 ray, glm::vec3 dir,
                     std::optional<glm::vec3> vertex_intersection = std::nullopt);

    RayCubeCollision(const RayCubeCollision &) = delete;

    RayCubeCollision(RayCubeCollision &&other) noexcept;

    ~RayCubeCollision() = default;

    RayCubeCollision &operator=(const RayCubeCollision &) = delete;
    RayCubeCollision &operator=(RayCubeCollision &&) = delete;

    [[nodiscard]] std::shared_ptr<T> cube() const noexcept {
        return m_cube;
    }

    [[nodiscard]] bool vertex_intersection() const noexcept {
        return m_vertex_intersection.has_value();
    }

    [[nodiscard]] const glm::vec3 &cube_intersection() const noexcept {
        return m_cube_intersection;
    }

    [[nodiscard]] const glm::vec3 &cube_face() const noexcept {
        return m_cube_face;
    }

    [[nodiscard]] const glm::vec3 &nearest_cube_corner() const noexcept {
        return m_nearest_cube_corner;
    }

    [[nodiscard]] const glm::vec3 &nearest_cube_edge() const noexcept {
        return m_nearest_cube_edge;
    }
};

} // namespace inexor::vulkan_renderer::world
