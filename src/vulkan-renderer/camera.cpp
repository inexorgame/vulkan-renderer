#include "inexor/vulkan-renderer/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace inexor::vulkan_renderer {
void Camera::update_view_matrix() {
    glm::mat4 rot_m = glm::mat4(1.0f);
    glm::mat4 trans_m;

    rot_m = glm::rotate(rot_m, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rot_m = glm::rotate(rot_m, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rot_m = glm::rotate(rot_m, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    trans_m = glm::translate(glm::mat4(1.0f), position * glm::vec3(1.0f, 1.0f, -1.0f));

    if (type == CameraType::FIRSTPERSON) {
        matrices.view = rot_m * trans_m;
    } else {
        matrices.view = trans_m * rot_m;
    }

    updated = true;
};

bool Camera::moving() {
    return keys.left || keys.right || keys.up || keys.down;
}

float Camera::get_near_clip() {
    return z_near;
}

float Camera::get_far_clip() {
    return z_far;
}

void Camera::set_perspective(float fov, float aspect, float z_near, float z_far) {
    this->fov = fov;
    this->z_near = z_near;
    this->z_far = z_far;
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
}

void Camera::update_aspect_ratio(float aspect) {
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
}

void Camera::set_position(glm::vec3 position) {
    this->position = position;
    update_view_matrix();
}

void Camera::set_rotation(glm::vec3 rotation) {
    this->rotation = rotation;
    update_view_matrix();
}

void Camera::rotate(glm::vec3 delta) {
    this->rotation += delta;
    update_view_matrix();
}

void Camera::set_translation(glm::vec3 translation) {
    this->position = translation;
    update_view_matrix();
}

void Camera::translate(glm::vec3 delta) {
    this->position += delta;
    update_view_matrix();
}

void Camera::update(float delta_time) {
    {
        updated = false;
        if (type == CameraType::FIRSTPERSON && moving()) {
            glm::vec3 cam_front;
            cam_front.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
            cam_front.y = sin(glm::radians(rotation.x));
            cam_front.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            cam_front = glm::normalize(cam_front);

            float move_speed = delta_time * movement_speed;

            if (keys.up) {
                position += cam_front * move_speed;
            }
            if (keys.down) {
                position -= cam_front * move_speed;
            }
            if (keys.left) {
                position -= glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
            }
            if (keys.right) {
                position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
            }

            update_view_matrix();
        }
    }
}

bool Camera::update_pad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time) {
    bool ret_val = false;

    if (type == CameraType::FIRSTPERSON) {
        // Use the common console thumbstick layout
        // Left = view, right = move

        const float dead_zone = 0.0015f;
        const float range = 1.0f - dead_zone;

        glm::vec3 cam_front;
        cam_front.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
        cam_front.y = sin(glm::radians(rotation.x));
        cam_front.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
        cam_front = glm::normalize(cam_front);

        float move_speed = delta_time * movement_speed * 2.0f;
        float rot_speed = delta_time * rotation_speed * 50.0f;

        // Move
        if (fabsf(axis_left.y) > dead_zone) {
            float pos = (fabsf(axis_left.y) - dead_zone) / range;
            position -= cam_front * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * move_speed;
            ret_val = true;
        }
        if (fabsf(axis_left.x) > dead_zone) {
            float pos = (fabsf(axis_left.x) - dead_zone) / range;
            position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * move_speed;
            ret_val = true;
        }

        // Rotate
        if (fabsf(axis_right.x) > dead_zone) {
            float pos = (fabsf(axis_right.x) - dead_zone) / range;
            rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * rot_speed;
            ret_val = true;
        }
        if (fabsf(axis_right.y) > dead_zone) {
            float pos = (fabsf(axis_right.y) - dead_zone) / range;
            rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * rot_speed;
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
