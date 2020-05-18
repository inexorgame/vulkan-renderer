#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer {

// TODO: Refactor method naming!
class Camera {
private:
    float fov;
    float z_near, z_far;

    void update_view_matrix();

public:
    enum CameraType { LOOKAT, FIRSTPERSON };
    CameraType type = CameraType::LOOKAT;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();

    float rotation_speed = 1.0f;
    float movement_speed = 1.0f;

    bool updated = false;

    struct {
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;

    struct {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys;

    bool moving();

    float get_near_clip();

    float get_far_clip();

    void set_perspective(float fov, float aspect, float z_near, float z_far);

    void update_aspect_ratio(float aspect);

    void set_position(glm::vec3 position);

    void set_rotation(glm::vec3 rotation);

    void rotate(glm::vec3 delta);

    void set_translation(glm::vec3 translation);

    void translate(glm::vec3 delta);

    void update(float delta_time);

    // Update camera passing separate axis data (gamepad)
    // Returns true if view or position has been changed
    bool update_pad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time);
};

} // namespace inexor::vulkan_renderer
