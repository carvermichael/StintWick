#if !defined(CAMERA)

#include "constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	glm::vec3 direction;

	float speed = 15.0f;

	float sensitivity = 0.1f;
	float yaw = -90.0f;
	float pitch = 45.0f;

	void initializeOverhead() {
		yaw = -90.0f;
		pitch = 0.0f;

		float midGridX = GRID_MAP_SIZE_X / 2;
		float midGridY = -GRID_MAP_SIZE_Y / 2;

		position = glm::vec3(midGridX, midGridY, 45.0f);

		up = glm::vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		glm::vec3 cameraRight = glm::normalize(glm::cross(up, direction));

		up = glm::normalize(glm::cross(direction, cameraRight));
	}

	void initializeForGrid() {
		yaw = -90.0f;
		pitch = 45.0f;

		float midGridX = GRID_MAP_SIZE_X / 2;
		float bottomGridY = -GRID_MAP_SIZE_Y * 2.0f;

		position = glm::vec3(midGridX, bottomGridY, GRID_MAP_SIZE_X * 1.5f);

		up = glm::vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		glm::vec3 cameraRight = glm::normalize(glm::cross(up, direction));

		up = glm::normalize(glm::cross(direction, cameraRight));
	}

	glm::mat4 generateView() {
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(direction);

		glm::mat4 view = glm::lookAt(position, position + front, up);
		return view;
	}

	void adjustYawAndPitch(float xOffset, float yOffset) {
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	void moveForward(float deltaTime) {
		position += speed * deltaTime * front;
	}

	void moveBack(float deltaTime) {
		position -= speed * deltaTime * front;
	}

	void moveLeft(float deltaTime) {
		position -= glm::normalize(glm::cross(front, up)) * (speed * deltaTime);
	}

	void moveRight(float deltaTime) {
		position += glm::normalize(glm::cross(front, up)) * (speed * deltaTime);
	}
};

#define CAMERA
#endif
