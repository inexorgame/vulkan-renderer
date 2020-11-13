#include "inexor/vulkan-renderer/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace inexor::vulkan_renderer {

Camera::Camera(const CameraType type, const glm::vec3 position, const glm::vec3 rotation, const float movement_speed,
               const float rotation_speed, const float fov, const float z_near, const float z_far,
               const std::uint32_t window_width, const std::uint32_t window_height)
    : m_type(type), m_position(position), m_rotation(rotation), m_movement_speed(movement_speed),
      m_rotation_speed(rotation_speed), m_fov(fov), m_z_near(z_near), m_z_far(z_far) {
    set_perspective(m_fov, static_cast<float>(window_width) / static_cast<float>(window_height), m_z_near, m_z_far);
    update_view_matrix();
}

void Camera::update_view_matrix() {
    glm::mat4 rot_m = glm::mat4(1.0f);
    glm::mat4 trans_m;

    rot_m = glm::rotate(rot_m, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rot_m = glm::rotate(rot_m, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rot_m = glm::rotate(rot_m, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    trans_m = glm::translate(glm::mat4(1.0f), m_position * glm::vec3(1.0f, 1.0f, -1.0f));

    if (m_type == CameraType::FIRSTPERSON) {
        m_matrices.view = rot_m * trans_m;
    } else {
        m_matrices.view = trans_m * rot_m;
    }

    m_updated = true;
};

bool Camera::moving() {
    return m_keys.left || m_keys.right || m_keys.up || m_keys.down;
}

float Camera::near_clip() {
    return m_z_near;
}

float Camera::far_clip() {
    return m_z_far;
}

void Camera::set_perspective(float fov, float aspect, float z_near, float z_far) {
    m_fov = fov;
    m_z_near = z_near;
    m_z_far = z_far;
    m_matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
}

void Camera::update_aspect_ratio(float aspect) {
    m_matrices.perspective = glm::perspective(glm::radians(m_fov), aspect, m_z_near, m_z_far);
}

void Camera::set_position(glm::vec3 position) {
    m_position = position;
    update_view_matrix();
}

void Camera::set_rotation(glm::vec3 rotation) {
    m_rotation = rotation;
    update_view_matrix();
}

void Camera::rotate(glm::vec3 delta) {
    m_rotation += delta;
    update_view_matrix();
}

void Camera::translate(glm::vec3 delta) {
    m_position += delta;
    update_view_matrix();
}

void Camera::update(float delta_time) {
    m_updated = false;
    if (m_type == CameraType::FIRSTPERSON && moving()) {
        glm::vec3 cam_front;
        cam_front.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
        cam_front.y = sin(glm::radians(m_rotation.x));
        cam_front.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
        cam_front = glm::normalize(cam_front);

        const float move_speed = delta_time * m_movement_speed;

        if (m_keys.up) {
            m_position += cam_front * move_speed;
        }
        if (m_keys.down) {
            m_position -= cam_front * move_speed;
        }
        if (m_keys.left) {
            m_position -= glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
        }
        if (m_keys.right) {
            m_position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
        }

        update_view_matrix();
    }
}

bool Camera::update_pad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time) {
    bool ret_val = false;

    if (m_type == CameraType::FIRSTPERSON) {
        // Use the common console thumbstick layout
        // Left = view, right = move

        const float dead_zone = 0.0015f;
        const float range = 1.0f - dead_zone;

        glm::vec3 cam_front;
        cam_front.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
        cam_front.y = sin(glm::radians(m_rotation.x));
        cam_front.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
        cam_front = glm::normalize(cam_front);

        const float move_speed = delta_time * m_movement_speed * 2.0f;
        const float rot_speed = delta_time * m_rotation_speed * 50.0f;

        // Move
        if (fabsf(axis_left.y) > dead_zone) {
            const float pos = (fabsf(axis_left.y) - dead_zone) / range;
            m_position -= cam_front * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * move_speed;
            ret_val = true;
        }
        if (fabsf(axis_left.x) > dead_zone) {
            const float pos = (fabsf(axis_left.x) - dead_zone) / range;
            m_position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * pos *
                          ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * move_speed;
            ret_val = true;
        }

        // Rotate
        if (fabsf(axis_right.x) > dead_zone) {
            const float pos = (fabsf(axis_right.x) - dead_zone) / range;
            m_rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * rot_speed;
            ret_val = true;
        }
        if (fabsf(axis_right.y) > dead_zone) {
            const float pos = (fabsf(axis_right.y) - dead_zone) / range;
            m_rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * rot_speed;
            ret_val = true;
        }
    } else {
        // todo: move code from example base class for look-at
    }

    if (ret_val) {
        update_view_matrix();
    }

    return ret_val;
}

} // namespace inexor::vulkan_renderer
