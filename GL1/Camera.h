#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float const CAM_MOUSE_SENSITIVITY = .1f;

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Forward;	// !!! It's the opposite of where the camera is looking at!!!
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Euler Angles
	float Pitch;
	float Yaw;

	// Options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
		float pitch = 0.0f,
		float yaw = -90.0f,
		float camSpeed = 2.5f,
		float zoom = 45.0f
	) :
		Position(position),
		WorldUp(glm::normalize(worldUp)),
		Forward(glm::normalize(direction)),
		Pitch(pitch),
		Yaw(yaw),
		MovementSpeed(camSpeed),
		MouseSensitivity(CAM_MOUSE_SENSITIVITY),
		Zoom(zoom)
	{
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Forward, Up);

		//float arr1[]{
		//	Right.x, Right.y, Right.z, 0,
		//	Up.x, Up.y, Up.z, 0,
		//	Forward.x, Forward.y, Forward.z, 0,
		//	0,0,0,1,
		//};
		//float arr2[]{
		//	1,0,0,-Position.x,
		//	0,1,0,-Position.y,
		//	0,0,1,-Position.z,
		//	0,0,0,1,
		//};
		//glm::mat4 mat1 = glm::transpose(glm::make_mat4(arr1));
		//glm::mat4 mat2 = glm::transpose(glm::make_mat4(arr2));
		//glm::mat4 lookAt = mat1 * mat2;
		//return lookAt;
	}

	void Translate(glm::vec3 dir, float deltaTime)
	{
		float v = MovementSpeed * deltaTime;
		dir = glm::normalize(dir);

		Position += (Forward)*dir.z *v;
		Position += (Up)*dir.y *v;
		Position += (Right)*dir.x *v;

	}

	void UpdateAngle(float xoffset, float yoffset)
	{
		xoffset *= MouseSensitivity * 0.1f;
		yoffset *= MouseSensitivity * 0.1f;

		Yaw += xoffset;
		Pitch += yoffset;

		// Clamp
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		updateCameraVectors();
	}

	void UpdateZoom(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

	void SetForward(glm::vec3 const & forward)
	{
		Forward = glm::normalize(forward);
		updateCameraVectors();
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Forward vector
		glm::vec3 direction;
		direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		direction.y = sin(glm::radians(Pitch));
		direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Forward = glm::normalize(direction);

		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Forward, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Forward));
	}
};