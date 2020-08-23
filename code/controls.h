#pragma once

#include "playerActions.h"

typedef void (*control)(int action, int key, float deltaTime);

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
void moveWithController(GLFWgamepadstate state, float deltaTime) {

    // movement
    float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]; 
    float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]; 

    if(glm::abs(leftX) < 0.1f) leftX = 0;
    if(glm::abs(leftY) < 0.1f) leftY = 0;

    // Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
    movePlayer(leftX, leftY * -1.0f);

    // movement
    float rightX = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]; 
    float rightY = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]; 

    if(glm::abs(rightX) < 0.1f) rightX = 0;
    if(glm::abs(rightY) < 0.1f) rightY = 0;

    // Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
    playerShoot(rightX, rightY * -1.0f, deltaTime);
}

void control_play(int action, int key, float deltaTime) {
	
}

void control_freeCam(int action, int key, float deltaTime) {
	if (action == GLFW_RELEASE) return;

	if (key == GLFW_KEY_L) {
		models.wallModel.scale(glm::vec3(0, 0, 1.1f));
	}
	
	if (key == GLFW_KEY_M) {
		models.wallModel.scale(glm::vec3(0, 0, 0.89f));
	}

	if (key == GLFW_KEY_G) {
		guidingGrid = !guidingGrid;
		addTextToBox("Guiding Grid: " + std::to_string(guidingGrid), &eventTextBox);
	}
	
	if (key == GLFW_KEY_R) {
		// Grid goes bye-bye when this happens.
		//regenerateMap(&world);
	}
	
	if (key == GLFW_KEY_O) {
		lightOrbit = !lightOrbit;
		addTextToBox("Light Orbit: " + std::to_string(lightOrbit), &eventTextBox);
	}
}

void control_edit(int action, int key, float deltaTime) {
	if (action != GLFW_PRESS) return;
	
	const float cameraSpeed = 5.0f * deltaTime;

	if (key == GLFW_KEY_W) {
		world.camera.moveForward(deltaTime);
	}
	if (key == GLFW_KEY_S) {
		world.camera.moveBack(deltaTime);
	}
	if (key == GLFW_KEY_A) {
		world.camera.moveLeft(deltaTime);
	}
	if (key == GLFW_KEY_D) {
		world.camera.moveRight(deltaTime);
	}
}

control getControlFunc() {
	if (mode == MODE_PLAY ||
		mode == MODE_PLAY_FIRST_PERSON)	return &control_play;
	if (mode == MODE_FREE_CAMERA)		return &control_freeCam;
	if (mode == MODE_LEVEL_EDIT)		return &control_edit;

	return NULL;
}
