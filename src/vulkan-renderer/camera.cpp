#include "inexor/vulkan-renderer/camera.hpp"

namespace inexor::vulkan_renderer {

void Camera::set_position(const glm::vec3 &position) {
    this->position = position;
    update_matrices();
}

void Camera::set_direction(const glm::vec3 &direction) {
    this->direction = direction;
    update_matrices();
}

glm::vec3 Camera::get_direction() const { return this->direction; }

void Camera::start_camera_movement(bool moving_backwards) {
    this->camera_is_moving = true;
    this->moving_backwards = moving_backwards;
}

void Camera::end_camera_movement() {
    this->camera_is_moving = false;
    this->moving_backwards = false;
}

void Camera::update(const float timestep) {
    this->timestep = timestep;

    if (camera_is_moving) {
        if (!moving_backwards) {
            move_forwards();
        } else {
            move_backwards();
        }

        update_matrices();
    }
}

void Camera::move_forwards() { this->position += (camera_speed * timestep * direction); }

void Camera::move_backwards() { this->position -= (camera_speed * timestep * direction); }

glm::vec3 Camera::get_position() const { return this->position; }

void Camera::set_yaw(const float yaw) { this->yaw = yaw; }

void Camera::set_pitch(const float pitch) { this->pitch = pitch; }

void Camera::set_roll(const float roll) { this->roll = roll; }

float Camera::get_yaw() const { return this->yaw; }

float Camera::get_pitch() const { return this->pitch; }

float Camera::get_roll() const { return this->roll; }

void Camera::set_rotation(const float yaw, const float pitch, const float roll) {
    this->yaw = yaw;
    this->pitch = pitch;
    this->roll = roll;
    // TODO: Update Matrices!
}

void Camera::move_camera_x(const float x) { this->position.x += (camera_speed * timestep * x); }

void Camera::move_camera_y(const float y) { this->position.y += (camera_speed * timestep * y); }

void Camera::move_camera_z(const float z) { this->position.z += (camera_speed * timestep * z); }

void Camera::set_speed(const float camera_speed) {
    assert(camera_speed > 0.0f);
    this->camera_speed = camera_speed;
    update_matrices();
}

float Camera::get_speed() const { return this->camera_speed; }

void Camera::set_near_plane(const float near_plane) {
    assert(near_plane > 0.0f);
    this->near_plane = near_plane;
}

float Camera::get_near_plane() const { return this->near_plane; }

void Camera::set_far_plane(const float far_plane) {
    assert(far_plane > 0.0f);
    this->far_plane = far_plane;
}

float Camera::get_far_plane() const { return this->far_plane; }

void Camera::set_zoom(const float zoom) {
    assert(zoom > 0.0f);
    this->zoom = zoom;
}

float Camera::get_zoom() const { return zoom; }

void Camera::set_aspect_ratio(const float aspect_ratio) {
    assert(aspect_ratio > 0.0f);
    this->aspect_ratio = aspect_ratio;
}

float Camera::get_aspect_ratio() const { return this->aspect_ratio; }

void Camera::update_matrices() {
    update_view_matrix();
    update_projection_matrix();
}

void Camera::update_view_matrix() {
    this->view_matrix = glm::lookAt(position, direction, world_up);
    ;
}

void Camera::update_projection_matrix() { this->projection_matrix = glm::perspective(glm::radians(45.0f), aspect_ratio, near_plane, far_plane); }

glm::mat4 Camera::get_view_matrix() { return this->view_matrix; }

glm::mat4 Camera::get_projection_matrix() { return this->projection_matrix; }

glm::vec3 Camera::get_up() const { return this->world_up; }

glm::vec3 Camera::get_front() const { return this->world_front; }

glm::vec3 Camera::get_right() const { return this->world_right; }

} // namespace inexor::vulkan_renderer
