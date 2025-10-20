#pragma once

#include <glm/vec3.hpp>

#include <string>
#include <tuple>

namespace inexor::vulkan_renderer::octree {

/// @brief A wrapper for collisions between a ray and octree geometry.
/// This class is used for octree collision, but it can be used for every cube-like data structure
/// @tparam T A template type which offers a size() and center() method.
template <typename T>
class RayCubeCollision {
    const T &m_cube;

    glm::vec3 m_intersection;
    glm::vec3 m_selected_face;
    glm::vec3 m_nearest_corner;
    glm::vec3 m_nearest_edge;

public:
    /// @brief Calculate point of intersection, selected face,
    /// nearest corner on that face, and nearest edge on that face.
    /// @param cube The cube to check for collision.
    /// @param ray_pos The start point of the ray.
    /// @param ray_dir The direction of the ray.
    RayCubeCollision(const T &cube, glm::vec3 ray_pos, glm::vec3 ray_dir);

    RayCubeCollision(const RayCubeCollision &) = delete;
    RayCubeCollision(RayCubeCollision &&other) noexcept;

    ~RayCubeCollision() = default;

    RayCubeCollision &operator=(const RayCubeCollision &) = delete;
    RayCubeCollision &operator=(RayCubeCollision &&) = delete;

    [[nodiscard]] const T &cube() const noexcept {
        return m_cube;
    }

    [[nodiscard]] const glm::vec3 &corner() const noexcept {
        return m_nearest_corner;
    }

    [[nodiscard]] const glm::vec3 &edge() const noexcept {
        return m_nearest_edge;
    }

    [[nodiscard]] const glm::vec3 &face() const noexcept {
        return m_selected_face;
    }

    [[nodiscard]] const glm::vec3 &intersection() const noexcept {
        return m_intersection;
    }
};

} // namespace inexor::vulkan_renderer::octree
