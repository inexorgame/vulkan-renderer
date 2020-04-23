#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>

#include <mutex>

namespace inexor::vulkan_renderer {

/// TODO: Add mutex!
/// TODO: Because this camera class will be used by scripting as well, runtime errors should be expected.
class Camera {
private:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

    float camera_speed = 1.0f;

    // TODO: Change with respect to window resolution!
    float aspect_ratio = 800 / 600;

    float yaw = 0.0f;

    float pitch = 0.0f;

    float roll = 0.0f;

    float near_plane = 0.1f;

    float far_plane = 10.0f;

    float zoom = 45.0f;

    bool camera_is_moving = false;

    bool moving_backwards = false;

    glm::vec3 world_up = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 world_front = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 world_right = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 view_matrix = glm::mat4();

    glm::mat4 projection_matrix = glm::mat4();

    /// Neccesary for taking into account the relative speed of the system's CPU.
    /// The timestep will be calculated in the main loop and will be passed on when update() is called.
    float timestep;

private:
    /// @brief Updates the view matrix.
    void update_view_matrix();

    /// @brief Updates the projection matrix.
    void update_projection_matrix();

public:
    Camera() = default;

    ~Camera() = default;

    /// @brief Updates all matrices.
    void update_matrices();

    /// @brief Start moving the camera every time update() is called.
    /// @param moving_backwards [in] True if the camera is moving backwards, false otherwise.
    void start_camera_movement(bool moving_backwards = false);

    /// @brief Ends moving the camera every time update() is called.
    void end_camera_movement();

    /// @brief Updates camera movement.
    /// @brief timestep [in] A float which scales with the amount of time which has passed since last rendering.
    void update(float timestep);

    /// @brief Sets the camera position.
    /// @param position [in] The position of the camera.
    void set_position(const glm::vec3 &position);

    /// @brief Returns he current camera position.
    glm::vec3 get_position() const;

    /// @brief Sets the relative speed of the camera.
    /// @param speed [in] The velocity of the camera movement.
    void set_speed(float camera_speed);

    /// @brief Returns the camera speed.
    float get_speed() const;

    /// @brief
    /// @param direction [in] The direction in which we look.
    void set_direction(const glm::vec3 &direction);

    /// @brief Returns the direction in which the camera is looking.
    glm::vec3 get_direction() const;

    /// @brief Moves the camera forwards with respect to the relative camera speed.
    void move_forwards();

    /// @brief Moves the camera backwards with respect to the relative camera speed.
    void move_backwards();

    /// @brief Moves the camera along the x-axis.
    /// @param y [in] The distance on the x-axis.
    void move_camera_x(float x);

    /// @brief Moves the camera along the y-axis.
    /// @param y [in] The distance on the y-axis.
    void move_camera_y(float y);

    /// @brief Moves the camera along the z-axis.
    /// @param y [in] The distance on the z-axis.
    void move_camera_z(float z);

    /// @brief Sets the yaw rotation angle.
    /// @param yaw [in] The yaw angle.
    void set_yaw(float yaw);

    /// @brief Sets the pitch rotation angle.
    /// @param pitch [in] The pitch angle.
    void set_pitch(float pitch);

    /// @brief Sets the roll rotation angle.
    /// @param roll [in] The roll angle.
    void set_roll(float roll);

    /// @brief Returns the yaw rotation angle.
    float get_yaw() const;

    /// @brief Returns the pitch rotation angle.
    float get_pitch() const;

    /// @brief Returns the roll rotation angle.
    float get_roll() const;

    /// @brief Sets the near plane for calculating the projection matrix.
    /// @param near_plane [in] The z-distance to the near plane.
    void set_near_plane(float near_plane);

    /// @brief Returns the near plane.
    float get_near_plane() const;

    /// @brief Sets the far plane for calculating the projection matrix.
    /// @param far_plane [in] The z-distance to the far plane.
    void set_far_plane(float far_plane);

    /// @brief Returns the far plane.
    float get_far_plane() const;

    /// @brief Sets the aspect ratio.
    /// @param aspect_ratio [in] The aspect ratio.
    void set_aspect_ratio(float aspect_ratio);

    /// @brief Returns the aspect ratio.
    float get_aspect_ratio() const;

    /// @brief Sets the rotation of the camera matrix.
    /// @param yaw [in] The yaw angle.
    /// @param pitch [in] The pitch angle.
    /// @param roll [in] The roll angle.
    void set_rotation(float yaw, float pitch, float roll);

    /// @brief Rotates the Camera around a certain center.
    /// @brief rotation_center [in] The center of rotation.
    /// @brief angle_x [in] The angle around x-axis.
    /// @brief angle_y [in] The angle around y-axis.
    /// @todo
    void rotate(const glm::vec3 &rotation_center, float angle_x, float angle_y);

    /// @brief Returns the rotation vector of the camera relative to the up vector.
    /// @todo
    glm::vec3 get_rotation() const;

    /// @brief Returns the up vector.
    glm::vec3 get_up() const;

    /// @brief Returns the front vector.
    glm::vec3 get_front() const;

    /// @brief Returns the right vector.
    glm::vec3 get_right() const;

    /// @brief Pan function (translate both camera eye and lookat point).
    /// @param x The angle on the x-axis.
    /// @param y The angle on the y-axis.
    /// @todo
    void pan(float x, float y);

    /// @brief Sets the zoom of the camera.
    /// @param zoom [in] The camera zoom.
    void set_zoom(float zoom);

    // TODO: min/max zoom!
    /// @brief Returns the camera zoom.
    float get_zoom() const;

    /// @brief Returns the view matrix.
    glm::mat4 get_view_matrix();

    /// @brief Returns the projection matrix.
    glm::mat4 get_projection_matrix();
};

} // namespace inexor::vulkan_renderer
