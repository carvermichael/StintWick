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

	my_vec3 posWithShake;
	float shakeAmount = 5.0f;
	float shakeTimeRemaining = 0.0f;

	void initOverhead(unsigned int gridSizeX, unsigned int gridSizeY) {
		yaw = -90.0f;
		pitch = 0.0f;

		float midGridX = (float) gridSizeX / 2;
		float midGridY = -(float) gridSizeY / 2;

		position = my_vec3(midGridX, midGridY, 80.0f);

		up = my_vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(radians(yaw)) * cos(radians(pitch));
		direction.y = sin(radians(pitch));
		direction.z = sin(radians(yaw)) * cos(radians(pitch));

		my_vec3 cameraRight = normalize(crossproduct(up, direction));

		up = normalize(crossproduct(direction, cameraRight));

		posWithShake = position;
	}

	void initForGrid(unsigned int gridSizeX, unsigned int gridSizeY) {
		yaw = -90.0f;
		pitch = 45.0f;

		float midGridX = (float)gridSizeX / 2;
		float bottomGridY = -(float)gridSizeY * 1.65f;

		position = my_vec3(midGridX, bottomGridY, (float) gridSizeY);

		up = my_vec3(0.0f, 1.0f, 0.0f);

		direction.x = cos(radians(yaw)) * cos(radians(pitch));
		direction.y = sin(radians(pitch));
		direction.z = sin(radians(yaw)) * cos(radians(pitch));

		my_vec3 cameraRight = normalize(crossproduct(up, direction));

		up = normalize(crossproduct(direction, cameraRight));

		posWithShake = position;
	}

	my_mat4 generateMyView() {
		direction.x = cos(radians(yaw)) * cos(radians(pitch));
		direction.y = sin(radians(pitch));
		direction.z = sin(radians(yaw)) * cos(radians(pitch));
		front = normalize(direction);

		my_mat4 view = lookAt(posWithShake, posWithShake + front, up);
		return view;
	}

	void update(float deltaTime) {
		// shake logic
		if (this->shakeTimeRemaining > 0.0f) {
			my_vec2 vec2 = randomVec2() * deltaTime * shakeAmount;

			this->posWithShake = this->position;
			this->posWithShake += normalize(crossproduct(front, up)) * vec2.x;
			this->posWithShake += vec2.y * up;

			this->shakeTimeRemaining -= deltaTime;
		}
	}

	void shakeScreen(float time) {
		this->shakeTimeRemaining = time;
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

	// For editing, moving along grid alignments for unit placement.
	// Grid size is one, so moving the amount of a normalized vector works here.
	void moveUpOne() {
		position += up;
	}
	void moveDownOne() {
		position -= up;
	}
	void moveLeftOne() {
		position -= normalize(crossproduct(front, up));
	}
	void moveRightOne() {
		position += normalize(crossproduct(front, up));
	}
};

#define CAMERA
#endif
