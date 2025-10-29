#include "inexor/vulkan-renderer/tools/camera.hpp"

namespace inexor::vulkan_renderer::tools {

Camera::Camera(const glm::vec3 &position, const float yaw, const float pitch, const float window_width,
               const float window_height)
    : m_position(position), m_yaw(yaw), m_pitch(pitch) {
    set_aspect_ratio(window_width, window_height);
    update_vectors();
    update_matrices();
}

void Camera::change_zoom(const float offset) {
    m_fov -= offset * m_zoom_step;

    // Make sure field of view is in range between specified minimum and maximum value.
    m_fov = std::clamp(m_fov, m_fov_min, m_fov_max);

    m_update_vertical_fov = true;
    m_update_perspective_matrix = true;
}

bool Camera::is_moving() const {
    return m_keys[0] || m_keys[1] || m_keys[2] || m_keys[3];
}

void Camera::rotate(const float delta_yaw, const float delta_pitch, const float delta_roll) {
    m_yaw += delta_yaw;
    m_yaw = std::fmod(m_yaw, 360.0f);
    m_pitch += delta_pitch;
    m_roll += delta_roll;
    m_pitch = std::clamp(m_pitch, m_pitch_min, m_pitch_max);
    m_update_view_matrix = true;
    update_vectors();
}

void Camera::set_aspect_ratio(const float width, const float height) {
    m_aspect_ratio = width / height;
    m_update_perspective_matrix = true;
    m_update_vertical_fov = true;
}

void Camera::set_far_plane(const float far_plane) {
    m_far_plane = far_plane;
    m_update_perspective_matrix = true;
}

void Camera::set_movement_speed(const float speed) {
    m_movement_speed = speed;
}

void Camera::set_movement_state(const CameraMovement key, const bool pressed) {
    switch (key) {
    case CameraMovement::FORWARD:
        m_keys[0] = pressed;
        break;
    case CameraMovement::LEFT:
        m_keys[1] = pressed;
        break;
    case CameraMovement::BACKWARD:
        m_keys[2] = pressed;
        break;
    case CameraMovement::RIGHT:
        m_keys[3] = pressed;
        break;
    }
}

void Camera::set_near_plane(const float near_plane) {
    m_near_plane = near_plane;
    m_update_perspective_matrix = true;
}

void Camera::set_position(const glm::vec3 position) {
    m_position = position;
    m_update_view_matrix = true;
    update_matrices();
    update_vectors();
}

void Camera::set_rotation(const float yaw, const float pitch, const float roll) {
    m_yaw = m_mouse_sensitivity * yaw;
    m_pitch = m_mouse_sensitivity * pitch;
    m_roll = m_mouse_sensitivity * roll;
    m_update_view_matrix = true;
    update_matrices();
    update_vectors();
}

void Camera::set_rotation_speed(const float speed) {
    m_rotation_speed = speed;
}

void Camera::set_type(const CameraType type) {
    m_type = type;
}

void Camera::update(const float delta_time) {
    if (m_type == CameraType::LOOK_AT && is_moving()) {
        update_vectors();
        if (is_moving()) {
            const float move_speed = delta_time * m_movement_speed;
            if (m_keys[0] && !m_keys[2]) {
                m_position += m_front * move_speed;
            }
            if (m_keys[1] && !m_keys[3]) {
                m_position -= m_right * move_speed;
            }
            if (m_keys[2] && !m_keys[0]) {
                m_position -= m_front * move_speed;
            }
            if (m_keys[3] && !m_keys[1]) {
                m_position += m_right * move_speed;
            }
        }
        m_update_view_matrix = true;
        update_matrices();
    }
}

void Camera::update_matrices() {
    if (m_type == CameraType::LOOK_AT) {
        if (m_update_vertical_fov) {
            m_vertical_fov = 2.0f * glm::atan(glm::tan(glm::radians(m_fov) / 2.0f) / m_aspect_ratio);
            m_update_vertical_fov = false;
        }
        if (m_update_view_matrix) {
            m_view_matrix = glm::lookAt(m_position, m_position + m_front, m_up);
            m_update_view_matrix = false;
        }
        if (m_update_perspective_matrix) {
            m_perspective_matrix = glm::perspective(m_vertical_fov, m_aspect_ratio, m_near_plane, m_far_plane);
            m_update_perspective_matrix = false;
        }
    }
}

void Camera::update_vectors() {
    if (m_type == CameraType::LOOK_AT) {
        glm::vec3 front;
        front.y = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        front.z = glm::sin(glm::radians(m_pitch));
        front.x = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));

        // Normalize the vectors, because their length gets closer to 0 the
        // more you look up or down which results in slower movement.
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_world_up));
        m_up = glm::normalize(glm::cross(m_right, m_front));

        m_update_view_matrix = true;
    }
}

} // namespace inexor::vulkan_renderer::tools
