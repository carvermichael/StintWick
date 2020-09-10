#pragma once

#include "playerActions.h"

typedef void (*control)(int action, int key, float deltaTime);

// NOTE: The check for GLFW_RELEASE relies on Windows repeat logic,
//		 probably don't want to rely on that long-term.		
//									-carver (8-10-20)
void moveWithController(GLFWgamepadstate state, float deltaTime) {
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
		addTextToBox("TimeStep: 1/" + std::to_string(timeStepDenom), &eventTextBox);
    }

    if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS &&  
       prevState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_RELEASE) {
        if(timeStepDenom > 1) {
            timeStepDenom--;
            addTextToBox("TimeStep: 1/" + std::to_string(timeStepDenom), &eventTextBox);
        }
    }

	if (state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS &&
		prevState.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_RELEASE) {
		pause = !pause;
		if (pause) {
			addTextToBox("Pause", &eventTextBox);
		} else {
			addTextToBox("Un-Pause", &eventTextBox);
		}
	}

    prevState = state;
}

void control_play(int action, int key, float deltaTime) {
	
}

void control_freeCam(int action, int key, float deltaTime) {
	if (action == GLFW_RELEASE) return;

	if (key == GLFW_KEY_G) {
		guidingGrid = !guidingGrid;
		addTextToBox("Guiding Grid: " + std::to_string(guidingGrid), &eventTextBox);
	}
	
	if (key == GLFW_KEY_O) {
		lightOrbit = !lightOrbit;
		addTextToBox("Light Orbit: " + std::to_string(lightOrbit), &eventTextBox);
	}
	if (key == GLFW_KEY_F2) {
		mode = MODE_PLAY;
		addTextToBox("Mode: Play", &eventTextBox);
	}


}

void control_edit(int action, int key, float deltaTime) {
	if (action != GLFW_PRESS) return;
	
	if (key == GLFW_KEY_F5) {
		saveAllLevels();
	}
	/*
	if (key == GLFW_KEY_RIGHT) {
		goForwardOneLevel();
	}
	if (key == GLFW_KEY_LEFT) {
		goBackOneLevel();
	}
	*/
	if (key == GLFW_KEY_UP) {
		world.camera.moveUpOne();
	}
	if (key == GLFW_KEY_DOWN) {
		world.camera.moveDownOne();
	}
	if (key == GLFW_KEY_LEFT) {
		world.camera.moveLeftOne();
	}
	if (key == GLFW_KEY_RIGHT) {
		world.camera.moveRightOne();
	}
	/*
	if (key == GLFW_KEY_EQUAL) {
		world.camera.moveForward(deltaTime);
	}
	if (key == GLFW_KEY_MINUS) {
		world.camera.moveBack(deltaTime);
	}
	*/
}

control getControlFunc() {
	if (mode == MODE_PLAY ||
		mode == MODE_PLAY_FIRST_PERSON)	return &control_play;
	if (mode == MODE_FREE_CAMERA)		return &control_freeCam;
	if (mode == MODE_LEVEL_EDIT)		return &control_edit;

	return NULL;
}
