#pragma once

#include "../time-step/inexor_time_step.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorCamera
	/// TODO: Add mutex.
	class InexorCamera
	{
		private:

			glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
			
			glm::vec3 direction = glm::vec3(2.0f, 2.0f, 2.0f);

			float camera_speed = 1.0f;

			// TODO: Parametrize!
			float aspect_ratio = 800/600;

			float fov = 90.0f;

			float yaw = 0.0f;

			float pitch = 0.0f;

			float roll = 0.0f;

			float near_plane = 0.1f;

			float far_plane = 10.0f;

			float zoom;

			InexorTimeStep timestep;

			glm::vec3 world_up = glm::vec3(0.0f, 0.0f, 1.0f);
			
			glm::vec3 world_front = glm::vec3(1.0f, 0.0f, 0.0f);
			
			glm::vec3 world_right = glm::vec3(0.0f, 1.0f, 0.0f);

			/// @TODO Only update when camera is moved.
			//void update_view_matrix();


		public:

			InexorCamera();
			
			
			~InexorCamera();


			/// @brief Sets the camera position.
			/// @param position [in] The position of the camera.
			void set_position(const glm::vec3& position);

			
			/// @brief Returns he current camera position.
			glm::vec3 get_position() const;


			/// @brief Sets the relative speed of the camera.
			/// @param speed [in] The velocity of the camera movement.
			void set_speed(const float camera_speed);


			/// @brief Returns the camera speed.
			float get_speed() const;


			/// @brief 
			/// @param direction [in] The direction in which we look.
			void set_direction(const glm::vec3& direction);


			/// @brief Returns the direction in which the camera is looking.
			glm::vec3 get_direction() const;


			/// @brief Moves the camera forwards with respect to the relative camera speed.
			void move_forwards();


			/// @brief Moves the camera backwards with respect to the relative camera speed.
			void move_backwards();

			
			/// @brief Moves the camera along the x-axis.
			/// @param y [in] The distance on the x-axis.
			void move_camera_x(const float x);

			
			/// @brief Moves the camera along the y-axis.
			/// @param y [in] The distance on the y-axis.
			void move_camera_y(const float y);

			
			/// @brief Moves the camera along the z-axis.
			/// @param y [in] The distance on the z-axis.
			void move_camera_z(const float z);

			
			/// @brief Sets the yaw rotation angle.
			/// @param yaw [in] The yaw angle.
			void set_yaw(const float yaw);

			
			/// @brief Sets the pitch rotation angle.
			/// @param pitch [in] The pitch angle.
			void set_pitch(const float pitch);

			
			/// @brief Sets the roll rotation angle.
			/// @param roll [in] The roll angle.
			void set_roll(const float roll);
			

			/// @brief Returns the yaw rotation angle.
			float get_yaw() const;

			
			/// @brief Returns the pitch rotation angle.
			float get_pitch() const;


			/// @brief Returns the roll rotation angle.
			float get_roll() const;


			/// @brief Sets the near plane for calculating the projection matrix.
			/// @param near_plane [in] The z-distance to the near plane.
			void set_near_plane(const float near_plane);
			
			
			/// @brief Returns the near plane.
			float get_near_plane() const;


			/// @brief Sets the far plane for calculating the projection matrix.
			/// @param far_plane [in] The z-distance to the far plane.
			void set_far_plane(const float far_plane);


			/// @brief Returns the far plane.
			float get_far_plane() const;


			/// @brief Sets the rotation of the camera matrix.
			/// @param yaw [in] The yaw angle.
			/// @param pitch [in] The pitch angle.
			/// @param roll [in] The roll angle.
			void set_rotation(const float yaw, const float pitch, const float roll);


			/// @brief Rotates the Camera around a certain center.
			/// @brief rotation_center [in] The center of rotation.
			/// @brief angle_x [in] The angle around x-axis.
			/// @brief angle_y [in] The angle around y-axis.
			/// @todo
			void rotate(const glm::vec3& rotation_center, float angle_x, float angle_y);

			
			/// @brief Returns the rotation vector of the camera relative to the up vector.
			/// @todo
			glm::vec3 get_rotation() const;


			/// @brief Returns the up vector.
			glm::vec3 get_up() const;


			/// 
			glm::vec3 get_front() const;


			/// 
			glm::vec3 get_right() const;


			/// 
			glm::mat4 get_matrix() const;

			
			/// @brief Pan function (translate both camera eye and lookat point).
			/// @param x The angle on the x-axis.
			/// @param y The angle on the y-axis.
			/// @todo
			void pan(const float x, const float y);


			/// @brief Sets the zoom of the camera.
			/// @param zoom [in] The camera zoom.
			void set_zoom(const float zoom);


			// TODO: min/max zoom!
			/// @brief Returns the camera zoom.
			float get_zoom() const;


			/// @brief Returns the view matrix.
			glm::mat4 get_view_matrix();


			/// @brief Returns the projection matrix.
			glm::mat4 get_projection_matrix();


	};


};
};
