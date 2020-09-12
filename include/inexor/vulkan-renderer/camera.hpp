#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer {

// TODO: Refactor method naming!
class Camera {
private:
    float m_fov;
    float m_z_near, m_z_far;

    /// @brief
    void update_view_matrix();

public:
    enum CameraType { LOOKAT, FIRSTPERSON };
    CameraType m_type{CameraType::LOOKAT};

    glm::vec3 m_rotation{};
    glm::vec3 m_position{};

    float m_rotation_speed{1.0f};
    float m_movement_speed{1.0f};

    bool m_updated{false};

    struct {
        glm::mat4 perspective;
        glm::mat4 view;
    } m_matrices;

    struct {
        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};
    } m_keys;

    /// @brief
    /// @return
    bool moving();

    /// @brief
    /// @return
    float near_clip();

    /// @brief
    /// @return
    float far_clip();

    /// @brief
    /// @param fov
    /// @param aspect
    /// @param z_near
    /// @param z_far
    void set_perspective(float fov, float aspect, float z_near, float z_far);

    /// @brief
    /// @param aspect
    void update_aspect_ratio(float aspect);

    /// @brief
    /// @param position
    void set_position(glm::vec3 position);

    /// @brief
    /// @param rotation
    void set_rotation(glm::vec3 rotation);

    /// @brief
    /// @param delta
    void rotate(glm::vec3 delta);

    /// @brief
    /// @param translation
    void set_translation(glm::vec3 translation);

    /// @brief
    /// @param delta
    void translate(glm::vec3 delta);

    /// @brief
    /// @param delta_time
    void update(float delta_time);

    /// @brief
    /// @param axis_left
    /// @param axis_right
    /// @param delta_time
    /// @return
    bool update_pad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time);
};

} // namespace inexor::vulkan_renderer
