#include "constants.h"
#include "camera.h"

void Camera::initOverhead() {
	yaw = -90.0f;
	pitch = 0.0f;
	 
	position = my_vec3(10.0f, 10.0f, 80.0f);

	up = my_vec3(0.0f, 1.0f, 0.0f);

	direction.x = cos(radians(yaw)) * cos(radians(pitch));
	direction.y = sin(radians(pitch));
	direction.z = sin(radians(yaw)) * cos(radians(pitch));

	my_vec3 cameraRight = normalize(crossproduct(up, direction));

	up = normalize(crossproduct(direction, cameraRight));

	posWithShake = position;
}

void Camera::initOnPlayer(my_vec3 playerPos) {
	yaw = -90.0f;
	pitch = 45.0f;
	speed = 45.0f;
	sensitivity = 0.1f;

	//float midGridX = (float)gridSizeX / 2;
	//float bottomGridY = -(float)gridSizeY * 1.65f;

	//position = my_vec3(midGridX, bottomGridY, (float) gridSizeY);

	//float midGridX = 15.0f;
	//float bottomGridY = -30.0f * 1.65f;

	//position = my_vec3(midGridX, bottomGridY, 30.0f);

	//update(0.0f, playerPos)
	
	float midGridX = playerPos.x;
	float bottomGridY = playerPos.y - 45.0f;

	position = my_vec3(midGridX, bottomGridY, 30.0f);

	up = my_vec3(0.0f, 1.0f, 0.0f);

	direction.x = cos(radians(yaw)) * cos(radians(pitch));
	direction.y = sin(radians(pitch));
	direction.z = sin(radians(yaw)) * cos(radians(pitch));

	my_vec3 cameraRight = normalize(crossproduct(up, direction));

	up = normalize(crossproduct(direction, cameraRight));

	posWithShake = position;
}

my_mat4 Camera::generateMyView() {
	direction.x = cos(radians(yaw)) * cos(radians(pitch));
	direction.y = sin(radians(pitch));
	direction.z = sin(radians(yaw)) * cos(radians(pitch));
	front = normalize(direction);

	// TODO: free camera breaks here when using posWithShake instead of just position, find out why
	my_mat4 view = lookAt(position, position + front, up);
	return view;
}

void Camera::update(float deltaTime, my_vec3 playerPos) {
	
	// find destination position
	// center x = playerPos.x
	// center y = playerPos.y - 45

	float squareCameraBoundsRadius = 2.5f;
	float cameraSpeedFast = 3.5f;
	float cameraSpeedSlow = 2.5f;

	float xDiff = position.x - playerPos.x;
	float absXDiff = xDiff < 0.0f ? xDiff * -1 : xDiff;

	float midGridX = 0.0f;
	if (absXDiff > squareCameraBoundsRadius) {
		midGridX = position.x - (xDiff / 1.5f) * cameraSpeedFast * deltaTime;
	}
	else {
		midGridX = position.x - (xDiff / 1.5f) * cameraSpeedSlow * deltaTime;
	}

	float yDiff = position.y - (playerPos.y - 45.0f);
	float absYDiff = yDiff < 0.0f ? yDiff * -1 : yDiff;

	float bottomGridY;
	if (absYDiff > squareCameraBoundsRadius) {
		bottomGridY = position.y - yDiff * cameraSpeedFast * deltaTime;
	}
	else {
		bottomGridY = position.y;
	}


	/*else {
		bottomGridY = position.y - yDiff * cameraSpeedSlow * deltaTime;
	}*/
	   	  
	// vec2OfCameraPos - vec2OfPlayerPos
	// if outside radius, move with ^^ that vec at faster speed
	// if inside radius, move with that vec at much slower speed


	// move to destination at camera speed (w/ timestep)
	   	 

	// simple movement with player
	//float midGridX = playerPos.x;
	//float bottomGridY = playerPos.y - 45.0f;

	position = my_vec3(midGridX, bottomGridY, 45.0f);

	direction.x = cos(radians(yaw)) * cos(radians(pitch));
	direction.y = sin(radians(pitch));
	direction.z = sin(radians(yaw)) * cos(radians(pitch));

	my_vec3 cameraRight = normalize(crossproduct(up, direction));

	up = normalize(crossproduct(direction, cameraRight));

	posWithShake = position;

	// shake logic
	if (this->shakeTimeRemaining > 0.00001f) {
		my_vec2 vec2 = randomVec2() * deltaTime * shakeAmount;

		this->posWithShake = this->position;
		this->posWithShake += normalize(crossproduct(front, up)) * vec2.x;
		this->posWithShake += vec2.y * up;

		this->shakeTimeRemaining -= deltaTime;
	}
}

void Camera::shakeScreen(float time) {
	this->shakeTimeRemaining = time;
}

void Camera::adjustYawAndPitch(float xOffset, float yOffset) {
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
}

void Camera::moveForward(float deltaTime) {
	position += front * speed * deltaTime;
}

void Camera::moveBack(float deltaTime) {
	position -= speed * deltaTime * front;
}

void Camera::moveUp(float deltaTime) {
	position += speed * deltaTime * up;
}

void Camera::moveDown(float deltaTime) {
	position -= speed * deltaTime * up;
}

void Camera::moveLeft(float deltaTime) {
	position -= normalize(crossproduct(front, up)) * (speed * deltaTime);
}

void Camera::moveRight(float deltaTime) {
	position += normalize(crossproduct(front, up)) * (speed * deltaTime);
}

// For editing, moving along grid alignments for unit placement.
// Grid size is one, so moving the amount of a normalized vector works here.
void Camera::moveUpOne() {
	position += up;
}
void Camera::moveDownOne() {
	position -= up;
}
void Camera::moveLeftOne() {
	position -= normalize(crossproduct(front, up));
}
void Camera::moveRightOne() {
	position += normalize(crossproduct(front, up));
}
