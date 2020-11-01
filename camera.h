#if !defined(CAMERA)

#include "constants.h"
#include "math.h"

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

	void initOverhead(unsigned int gridSizeX, unsigned int gridSizeY);

	void initForGrid(unsigned int gridSizeX, unsigned int gridSizeY, my_vec3 playerPos);

	my_mat4 generateMyView();

	void update(float deltaTime, my_vec3 playerPos);

	void shakeScreen(float time);

	void adjustYawAndPitch(float xOffset, float yOffset);

	void moveForward(float deltaTime);
	void moveBack(float deltaTime);

	void moveUp(float deltaTime);
	void moveDown(float deltaTime);
	void moveLeft(float deltaTime);
	void moveRight(float deltaTime);

	// For editing, moving along grid alignments for unit placement.
	// Grid size is one, so moving the amount of a normalized vector works here.
	void moveUpOne();
	void moveDownOne();
	void moveLeftOne();
	void moveRightOne();
};

#define CAMERA
#endif
