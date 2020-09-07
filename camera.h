#if !defined(CAMERA)

#include "constants.h"
#include "math.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
	my_vec3 position;
	my_vec3 front;
	my_vec3 up;

	my_vec3 direction;

	float speed = 45.0f;

	float sensitivity = 0.1f;
	float yaw = -90.0f;
	float pitch = 45.0f;

	void initOverhead(unsigned int gridSizeX, unsigned int gridSizeY) {
		yaw = -90.0f;
		pitch = 0.0f;

		float midGridX = (float) gridSizeX / 2;
		float midGridY = -(float) gridSizeY / 2;

		position = my_vec3(midGridX, midGridY, 80.0f);

		up = my_vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		my_vec3 cameraRight = normalize(crossproduct(up, direction));

		up = normalize(crossproduct(direction, cameraRight));
	}

	void initForGrid(unsigned int gridSizeX, unsigned int gridSizeY) {
		yaw = -90.0f;
		pitch = 45.0f;

		float midGridX = (float) gridSizeX / 2;
		float bottomGridY = -(float)gridSizeY * 1.65f;

		position = my_vec3(midGridX, bottomGridY, (float) gridSizeY);

		up = my_vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		my_vec3 cameraRight = normalize(crossproduct(up, direction));

		up = normalize(crossproduct(direction, cameraRight));
	}

	glm::mat4 generateView() {
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = normalize(direction);

		glm::mat4 view = glm::lookAt(toGLM(position), toGLM(position + front), toGLM(up));
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
		position += front * speed * deltaTime;
	}

	void moveBack(float deltaTime) {
		position -= speed * deltaTime * front;
	}

	void moveUp(float deltaTime) {
		position += speed * deltaTime * up;
	}

	void moveDown(float deltaTime) {
		position -= speed * deltaTime * up;
	}

	void moveLeft(float deltaTime) {
		position -= normalize(crossproduct(front, up)) * (speed * deltaTime);
	}

	void moveRight(float deltaTime) {
		position += normalize(crossproduct(front, up)) * (speed * deltaTime);
	}
};

#define CAMERA
#endif
