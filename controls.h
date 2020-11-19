#pragma once

#include "playerActions.h"

typedef void (*control)(int action, int key, float deltaTime);

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
inline void moveWithController(GLFWgamepadstate state, float deltaTime) {
    static GLFWgamepadstate prevState;

    // movement
    float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]; 
    float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]; 

    if(glm::abs(leftX) < 0.1f) leftX = 0;
    if(glm::abs(leftY) < 0.1f) leftY = 0;

	leftX *= deltaTime;
	leftY *= deltaTime;

    // Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
    movePlayer(leftX, leftY * -1.0f);

    // movement
    float rightX = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]; 
    float rightY = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]; 

    if(glm::abs(rightX) < 0.1f) rightX = 0;
    if(glm::abs(rightY) < 0.1f) rightY = 0;

    // Up on Y joystick is negative. Flipping here to make it easier to work with in relation to world space.
    playerShoot(rightX, rightY * -1.0f, deltaTime);

    if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS &&  
       prevState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_RELEASE) {
        timeStepDenom++;
		eventTextBox.addTextToBox("TimeStep: 1/" + std::to_string(timeStepDenom));
    }

    if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS &&  
       prevState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_RELEASE) {
        if(timeStepDenom > 1) {
            timeStepDenom--;
			eventTextBox.addTextToBox("TimeStep: 1/" + std::to_string(timeStepDenom));
        }
    }

    prevState = state;
}

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
	if (key == GLFW_KEY_F2) {
		mode = MODE_PLAY;
		eventTextBox.addTextToBox("Mode: Play");
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
		world->camera.moveUpOne();
	}
	if (key == GLFW_KEY_DOWN) {
		world->camera.moveDownOne();
	}
	if (key == GLFW_KEY_LEFT) {
		world->camera.moveLeftOne();
	}
	if (key == GLFW_KEY_RIGHT) {
		world->camera.moveRightOne();
	}
	
	if (key == GLFW_KEY_EQUAL) {
		world->camera.moveForward(deltaTime);
	}
	if (key == GLFW_KEY_MINUS) {
		world->camera.moveBack(deltaTime);
	}
	
}

inline control getControlFunc() {
	if (mode == MODE_PLAY)				return &control_play;
	if (mode == MODE_FREE_CAMERA)		return &control_freeCam;
	if (mode == MODE_LEVEL_EDIT)		return &control_edit;

	return NULL;
}

inline void processKeyboardInput(GLFWwindow *window, float deltaTime) {
	// NOTE: Using the callback for free camera movement is super choppy,
	//		 Cause it's the only thing that involves holding down keys?
	if (mode == MODE_FREE_CAMERA) {
		const float cameraSpeed = 25.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			world->camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			world->camera.moveBack(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			world->camera.moveLeft(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			world->camera.moveRight(deltaTime);
		}
	}
	else if (mode == MODE_LEVEL_EDIT) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			world->camera.moveUpOne();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			world->camera.moveDownOne();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			world->camera.moveLeftOne();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			world->camera.moveRightOne();
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			world->camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			world->camera.moveBack(deltaTime);
		}
	}
}

inline void processJoystickInput(GLFWgamepadstate state, float deltaTime) {
	if (mode != MODE_PLAY && mode != MODE_REPLAY) return;

	moveWithController(state, deltaTime);
}
