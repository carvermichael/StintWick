#pragma once

#include "playerActions.h"

typedef void (*control)(int action, int key, float deltaTime);

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
inline void control_play(int action, int key, float deltaTime) {
	
}

inline void control_freeCam(int action, int key, float deltaTime) {
	if (action == GLFW_RELEASE) return;

	if (key == GLFW_KEY_G) {
		guidingGrid = !guidingGrid;
		eventTextBox.addTextToBox("Guiding Grid: " + std::to_string(guidingGrid));
	}
	
	if (key == GLFW_KEY_O) {
		lightOrbit = !lightOrbit;
		eventTextBox.addTextToBox("Light Orbit: " + std::to_string(lightOrbit));
	}
}

inline void control_edit(int action, int key, float deltaTime) {
	if (action != GLFW_PRESS) return;
	
	if (key == GLFW_KEY_F5) {
		saveAllLevelsV2(levels, levelCount, &eventTextBox);
	}
	
	if (key == GLFW_KEY_Q) {
		goForwardOneLevel();
	}
	if (key == GLFW_KEY_E) {
		goBackOneLevel();
	}
	
	if (key == GLFW_KEY_UP) {
		camera.moveUpOne();
	}
	if (key == GLFW_KEY_DOWN) {
		camera.moveDownOne();
	}
	if (key == GLFW_KEY_LEFT) {
		camera.moveLeftOne();
	}
	if (key == GLFW_KEY_RIGHT) {
		camera.moveRightOne();
	}
	
	if (key == GLFW_KEY_EQUAL) {
		camera.moveForward(deltaTime);
	}
	if (key == GLFW_KEY_MINUS) {
		camera.moveBack(deltaTime);
	}
	
}

inline control getControlFunc() {
	if (globalMode == MODE_PLAY)				return &control_play;
	if (globalMode == MODE_FREE_CAMERA)		return &control_freeCam;
	if (globalMode == MODE_LEVEL_EDIT)		return &control_edit;

	return NULL;
}

inline void processKeyboardInput(GLFWwindow *window, float deltaTime) {
	// NOTE: Using the callback for free camera movement is super choppy,
	//		 Cause it's the only thing that involves holding down keys?
	if (globalMode == MODE_FREE_CAMERA) {
		const float cameraSpeed = 25.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.moveBack(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.moveLeft(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.moveRight(deltaTime);
		}
	}
	else if (globalMode == MODE_LEVEL_EDIT) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.moveUpOne();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.moveDownOne();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.moveLeftOne();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.moveRightOne();
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			camera.moveBack(deltaTime);
		}
	}
}

