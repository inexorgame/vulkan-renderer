#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer {

/// @brief A RAII wrapper class for camera position and movement.
/// @todo Refactor method naming!
class Camera {
    float m_fov{0.0f};
    float m_z_near{0.0f};
    float m_z_far{0.0f};

    /// @brief Update the view matrix.
    /// @note The view matrix should only be updated if necessary.
    void update_view_matrix();

public:
    // TODO: Why is so much stuff here public? refactor!

    enum class CameraType { LOOKAT, FIRSTPERSON };
    CameraType m_type{CameraType::LOOKAT};

    glm::vec3 m_rotation{};
    glm::vec3 m_position{};

    float m_rotation_speed{1.0f};
    float m_movement_speed{1.0f};

    bool m_updated{false};

    struct {
        glm::mat4 perspective = glm::mat4();
        glm::mat4 view = glm::mat4();
    } m_matrices;

    struct {
        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};
    } m_keys;

    /// @brief Check if any of the following keys is pressed: W, A, S, D.
    bool moving();

    /// @brief Return the near z distance.
    float near_clip();

    /// @brief Return the far z distance.
    float far_clip();

    /// @brief Sets the perspective parameters.
    /// @param fov Field of view, 90 by default.
    /// @param aspect The aspect ratio.
    /// @param z_near The near z distance.
    /// @param z_far The far z distance.
    void set_perspective(float fov, float aspect, float z_near, float z_far);

    /// @brief Update the aspect ratio.
    /// This method will for example be called if the window aspect ratio changes
    /// @param aspect The aspect ratio.
    void update_aspect_ratio(float aspect);

    /// @brief Set the camera's position.
    /// @param position The camera position.
    void set_position(glm::vec3 position);

    /// @brief Set the camera's rotation.
    /// @param rotation The rotation axis.
    void set_rotation(glm::vec3 rotation);

    /// @brief Rotate the camera a certain amount.
    /// @param delta The camera rotation.
    void rotate(glm::vec3 delta);

    void translate(glm::vec3 delta);

    /// @brief Update the camera's position and rotation.
    /// @param delta_time The amount of time which passed since last update.
    void update(float delta_time);

    /// @brief Update camera passing separate axis data (gamepad).
    /// @param axis_left The change in axis 1.
    /// @param axis_right The change in axis 2.
    /// @param delta_time The amount of time which passed since last update.
    /// @return Returns true if view or position has been changed.
    bool update_pad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time);
};

} // namespace inexor::vulkan_renderer
