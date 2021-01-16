#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <vector>

namespace inexor::vulkan_renderer {

namespace directions {
/// The default value of the camera's front vector.
constexpr glm::vec3 DEFAULT_FRONT{1.0f, 0.0f, 0.0f};
/// The default value of the camera's right vector.
constexpr glm::vec3 DEFAULT_RIGHT{0.0f, 1.0f, 0.0f};
/// The default value of the camera's up vector.
constexpr glm::vec3 DEFAULT_UP{0.0f, 0.0f, 1.0f};
}; // namespace directions

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

// TODO: Implement more camera types.
enum class CameraType { LOOK_AT };

/// @warning Not thread safe!
class Camera {
private:
    /// The type of the camera. Currently only one type is implemented.
    CameraType m_type{CameraType::LOOK_AT};
    /// The start position of the camera.
    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    /// The vector of direction in which the camera is looking.
    glm::vec3 m_front{directions::DEFAULT_FRONT};
    /// The vector of direction which points to the right.
    glm::vec3 m_right{directions::DEFAULT_RIGHT};
    /// The vector which indicates "upwards".
    glm::vec3 m_up{directions::DEFAULT_UP};
    /// The world vector which indicates "upwards".
    glm::vec3 m_world_up{directions::DEFAULT_UP};
    glm::mat4 m_view_matrix;
    glm::mat4 m_perspective_matrix;

    /// The camera's yaw angle.
    float m_yaw{0.0f};
    /// The camera's roll angle.
    float m_roll{0.0f};
    /// The camera's pitch angle.
    float m_pitch{0.0f};
    /// The camera's minimum pitch angle.
    /// Looking straight downwards is the maximum pitch angle.
    float m_pitch_min{-89.0f};
    /// The camera's maximum pitch angle.
    /// Looking straight upwards is the maximum pitch angle.
    float m_pitch_max{+89.0f};
    /// The camera's field of view.
    float m_fov{90.0f};
    /// The camera's maximum field of view.
    float m_fov_max{90.0f};
    /// The camera's minimum field of view.
    float m_fov_min{20.0f};
    /// The zoom step when zooming in or out.
    float m_zoom_step{10.0f};
    /// The camera's rotation speed.
    float m_rotation_speed{1.0f};
    /// The camera's movement speed.
    float m_movement_speed{2.0f};
    /// The camera's aspect ratio (width divided by height).
    float m_aspect_ratio{1920.0f / 1080.0f};
    /// The sensivity of the mouse.
    float m_mouse_sensitivity{0.005f};
    /// The camera's near plane.
    float m_near_plane{0.001f};
    /// The camera's far plane.
    float m_far_plane{1000.0f};

    /// The keys for the movement FORWARD, BACKWARD, LEFT, RIGHT.
    std::array<bool, 4> m_keys{false, false, false, false};
    /// Will be set to ``true`` if the matrices need to be recalculated.
    bool m_update_needed{true};

    void update_vectors();

    void update_matrices();

    [[nodiscard]] bool is_moving() const;

public:
    /// @brief Default constructor.
    /// @param position The camera's position.
    /// @param yaw The camera's yaw angle in degrees.
    /// @param pitch The camera's pitch angle in degrees.
    /// @param window_width The width of the window.
    /// @param window_height The height of the window.
    Camera(const glm::vec3 &position, float yaw, float pitch, float window_width, float window_height);

    // TODO: Add more overloaded constructors.

    /// @brief Set the camera type.
    /// @note We will implement more camera types in the future.
    /// @param type The camera type.
    void set_type(CameraType type);

    [[nodiscard]] const CameraType &type() const {
        return m_type;
    }

    /// @brief Notify the camera if a certain key is pressed or released.
    /// @param key The key which was pressed or released.
    /// @param pressed ``true`` if the key is pressed.
    void set_movement_state(CameraMovement key, bool pressed);

    /// @brief Set the position of the camera.
    /// @param position The position of the camera.
    void set_position(glm::vec3 position);

    [[nodiscard]] const glm::vec3 &position() const {
        return m_position;
    }

    /// @brief Set the aspect ratio (window width divided by window height) of the camera view matrix.
    /// @param width The width of the window.
    /// @param height The height of the window.
    void set_aspect_ratio(float width, float height);

    [[nodiscard]] float aspect_ratio() const {
        return m_aspect_ratio;
    }

    [[nodiscard]] float fov() const {
        return m_fov;
    }

    /// @brief Set the movement speed of the camera.
    /// @param speed The movement speed of the camera.
    void set_movement_speed(float speed);

    [[nodiscard]] float movement_speed() const {
        return m_movement_speed;
    }

    /// @brief Set the rotation speed of the camera.
    /// @param speed The rotation speed of the camera.
    void set_rotation_speed(float speed);

    [[nodiscard]] float rotation_speed() const {
        return m_rotation_speed;
    }

    /// @brief Rotates the camera around x, y, and z axis.
    /// @param delta_yaw The yaw angle.
    /// @param delta_pitch The pitch angle.
    /// @param delta_roll The roll angle.
    void rotate(float delta_yaw, float delta_pitch, float delta_roll = 0.0f);

    /// @brief Set the camera's rotation.
    /// @param yaw The yaw angle.
    /// @param pitch The pitch angle.
    /// @param roll The roll angle.
    void set_rotation(float yaw, float pitch, float roll);

    [[nodiscard]] const glm::vec3 &rotation() const {
        return m_front;
    }

    [[nodiscard]] float yaw() const {
        return m_yaw;
    }

    [[nodiscard]] float pitch() const {
        return m_pitch;
    }

    [[nodiscard]] float roll() const {
        return m_roll;
    }

    [[nodiscard]] const glm::vec3 &front() const {
        return m_front;
    }

    [[nodiscard]] const glm::vec3 &up() const {
        return m_up;
    }

    [[nodiscard]] const glm::vec3 &right() const {
        return m_right;
    }

    /// @brief Set the near plane distance of the camera.
    /// @param near_plane The near plane distance.
    void set_near_plane(float near_plane);

    [[nodiscard]] float near_plane() const {
        return m_near_plane;
    }

    /// @brief Set the far plane distance of the camera.
    /// @param far_plane The far plane distance.
    void set_far_plane(float far_plane);

    [[nodiscard]] float far_plane() const {
        return m_far_plane;
    }

    /// @brief Change the zoom of the camera.
    /// @param offset The mouse wheel offset change.
    void change_zoom(float offset);

    /// @brief Update the camera (recalculate vectors and matrices).
    /// @param delta_time The change in time since the last frame.
    void update(float delta_time);

    [[nodiscard]] const glm::mat4 &view_matrix() {
        if (m_update_needed) {
            update_matrices();
        }
        return m_view_matrix;
    }

    [[nodiscard]] const glm::mat4 &perspective_matrix() {
        if (m_update_needed) {
            update_matrices();
        }
        return m_perspective_matrix;
    }
};
}; // namespace inexor::vulkan_renderer
