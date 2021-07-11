#include <inexor/vulkan-renderer/world/collision.hpp>

#include <inexor/vulkan-renderer/world/cube.hpp>

#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <array>
#include <limits>
#include <memory>
#include <utility>

namespace inexor::vulkan_renderer::world {

template <typename T>
RayCubeCollision<T>::RayCubeCollision(RayCubeCollision &&other) noexcept : m_cube{other.m_cube} {
    m_intersection = other.m_intersection;
    m_selected_face = other.m_selected_face;
    m_nearest_corner = other.m_nearest_corner;
    m_nearest_edge = other.m_nearest_edge;
}

template <typename T>
RayCubeCollision<T>::RayCubeCollision(const T &cube, const glm::vec3 ray_pos, const glm::vec3 ray_dir) : m_cube(cube) {

    /// In order to work with cubes of arbitrary size, this lambda calculates the center of a cube's face with respect
    /// to the size of the octree.
    const auto adjust_coordinates = [=](const glm::vec3 pos) {
        // TODO: Take rotation of the cube into account.
        return m_cube.center() + pos * (m_cube.size() / 2);
    };

    /// x: left/right, y: front/back, z: top/bottom
    static constexpr std::array BBOX_DIRECTIONS{
        glm::vec3(-1.0f, 0.0f, 0.0f), // left
        glm::vec3(1.0f, 0.0f, 0.0f),  // right
        glm::vec3(0.0f, -1.0f, 0.0f), // front
        glm::vec3(0.0f, 1.0f, 0.0f),  // back
        glm::vec3(0.0f, 0.0f, 1.0f),  // top
        glm::vec3(0.0f, 0.0f, -1.0f)  // bottom
    };

    /// The coordinates of the center of every face of the cube.
    const std::array bbox_face_centers{adjust_coordinates(BBOX_DIRECTIONS[0]),  // left
                                       adjust_coordinates(BBOX_DIRECTIONS[1]),  // right
                                       adjust_coordinates(BBOX_DIRECTIONS[2]),  // front
                                       adjust_coordinates(BBOX_DIRECTIONS[3]),  // back
                                       adjust_coordinates(BBOX_DIRECTIONS[4]),  // top
                                       adjust_coordinates(BBOX_DIRECTIONS[5])}; // bottom

    const std::array bbox_corners{adjust_coordinates({-1.0f, -1.0f, -1.0f}), // left front bottom
                                  adjust_coordinates({-1.0f, -1.0f, 1.0f}),  // left front top
                                  adjust_coordinates({-1.0f, 1.0f, -1.0f}),  // left back bottom
                                  adjust_coordinates({-1.0f, 1.0f, 1.0f}),   // left back top
                                  adjust_coordinates({1.0f, -1.0f, -1.0f}),  // right front bottom
                                  adjust_coordinates({1.0f, -1.0f, 1.0f}),   // right front top
                                  adjust_coordinates({1.0f, 1.0f, 1.0f}),    // right back bottom
                                  adjust_coordinates({1.0f, 1.0f, -1.0f})};  // right back top

    using bbox_corner_on_face_index = std::array<std::size_t, 4>;

    /// These indices specify which 4 bounding box corner points are the corners on the given face.
    static constexpr std::array BBOX_CORNERS_ON_FACE_INDICES{
        bbox_corner_on_face_index{0, 1, 2, 3}, // left
        bbox_corner_on_face_index{4, 5, 6, 7}, // right
        bbox_corner_on_face_index{0, 1, 4, 5}, // front
        bbox_corner_on_face_index{2, 3, 6, 7}, // back
        bbox_corner_on_face_index{1, 3, 5, 7}, // top
        bbox_corner_on_face_index{0, 2, 4, 6}  // bottom
    };

    /// The coordinates of the center of every edge of the cube.
    const std::array bbox_edges{adjust_coordinates({-1.0f, 0.0f, 1.0f}),  // left top
                                adjust_coordinates({-1.0f, 1.0f, 0.0f}),  // left front
                                adjust_coordinates({-1.0f, 0.0f, -1.0f}), // left bottom
                                adjust_coordinates({-1.0f, -1.0f, 0.0f}), // left back
                                adjust_coordinates({1.0f, 0.0f, 1.0f}),   // right top
                                adjust_coordinates({1.0f, 1.0f, 0.0f}),   // right front
                                adjust_coordinates({1.0f, 0.0f, -1.0f}),  // right bottom
                                adjust_coordinates({1.0f, -1.0f, 0.0f}),  // right back
                                adjust_coordinates({0.0f, -1.0f, -1.0f}), // middle bottom front
                                adjust_coordinates({0.0f, 1.0f, -1.0f}),  // middle bottom back
                                adjust_coordinates({0.0f, -1.0f, 1.0f}),  // middle top front
                                adjust_coordinates({0.0f, 1.0f, 1.0f})};  // middle top back

    using edge_on_face_index = std::array<std::size_t, 4>;

    /// These indices specify which 4 edges are associated with a given face of the bounding box.
    static constexpr std::array BBOX_EDGE_ON_FACE_INDICES{
        edge_on_face_index{0, 1, 2, 3},   // left
        edge_on_face_index{4, 5, 6, 7},   // right
        edge_on_face_index{1, 5, 8, 11},  // front
        edge_on_face_index{3, 7, 9, 11},  // back
        edge_on_face_index{0, 4, 10, 11}, // top
        edge_on_face_index{2, 6, 8, 9}    // bottom
    };

    /// @brief Determine the intersection point between a ray and a plane.
    /// @note This function is not implemented by default in glm
    /// @param plane_pos The position of the plane
    /// @param plane_norm The normal vector of the plane
    /// @param ray_pos Point a on the ray
    /// @param ray_dir Point b on the ray
    const auto ray_plane_intersection_point = [&](const glm::vec3 &plane_pos, const glm::vec3 &plane_norm,
                                                  const glm::vec3 &ray_pos, const glm::vec3 &ray_dir) {
        return ray_pos - ray_dir * (glm::dot((ray_pos - plane_pos), plane_norm) / glm::dot(ray_dir, plane_norm));
    };

    /// @brief As soon as we determined which face is selected and we calculated the intersection point between the
    /// ray and the face plane, we need to determine the nearest corner in that face and the nearest edge on that
    /// face. In order to determine the nearest corner on a selected face for example, we would iterate through all
    /// corners on the selected face and calculate the distance between the intersection point and the corner's
    /// coordinates. The corner which is closest to the intersection point is the selected corner.
    /// However, we should not use ``glm::distance`` for this, because it performs a ``sqrt`` calculation on the
    /// vector coordinates. This is not necessary in this case, as we are not interested in the exact distance but
    /// rather in a value which allows us to determine the nearest corner. This means we can use the squared
    /// distance, which allows us to avoid the costly call of ``sqrt``.
    /// @param points A const reference to a std::array of size 2 which contains both points
    const auto square_of_distance = [&](const std::array<glm::vec3, 2> &points) {
        return glm::distance2(points[0], points[1]);
    };

    float shortest_squared_distance{std::numeric_limits<float>::max()};
    float squared_distance{std::numeric_limits<float>::max()};

    /// The index of the array which contains the coordinates of the face centers of the cube's bounding box.
    std::size_t selected_face_index{0};

    // Loop though all faces of the cube and check for collision between ray and face plane
    // There is no need to sort the data as we are interested in the smallest value only.
    for (std::size_t i = 0; i < 6; i++) {

        // Check if the cube side is facing the camera: if the dot product of the two vectors is smaller than
        // zero, the corresponding angle is smaller than 90 degrees, so the side is facing the camera. Check the
        // references page or math books about geometry for a detailed explanation of this equation.
        if (glm::dot(BBOX_DIRECTIONS[i], ray_dir) < 0.0f) {

            const auto intersection =
                ray_plane_intersection_point(bbox_face_centers[i], BBOX_DIRECTIONS[i], ray_pos, ray_dir);

            squared_distance = square_of_distance({m_cube.center(), intersection});

            if (squared_distance < shortest_squared_distance) {
                selected_face_index = i;
                shortest_squared_distance = squared_distance;
                m_intersection = intersection;
                m_selected_face = bbox_face_centers[i];
            }
        }
    }

    // Reset value to maximum for the search of the closest corner.
    shortest_squared_distance = std::numeric_limits<float>::max();

    // Loop through all corners of this face and check for the nearest one.
    for (const auto corner_index : BBOX_CORNERS_ON_FACE_INDICES[selected_face_index]) {
        squared_distance = square_of_distance({bbox_corners[corner_index], m_intersection});

        if (squared_distance < shortest_squared_distance) {
            shortest_squared_distance = squared_distance;
            m_nearest_corner = bbox_corners[corner_index];
        }
    }

    // Reset value to maximum for the search of the closest edge.
    shortest_squared_distance = std::numeric_limits<float>::max();

    // Iterate through all edges on this face and select the nearest one.
    for (const auto edge_index : BBOX_EDGE_ON_FACE_INDICES[selected_face_index]) {

        squared_distance = square_of_distance({bbox_edges[edge_index], m_intersection});

        if (squared_distance < shortest_squared_distance) {
            shortest_squared_distance = squared_distance;
            m_nearest_edge = bbox_edges[edge_index];
        }
    }
}

// Explicit instantiation
template RayCubeCollision<Cube>::RayCubeCollision(const Cube &, const glm::vec3, const glm::vec3);

} // namespace inexor::vulkan_renderer::world
