#pragma once

typedef void (*control)(int action, int key);

// NOTE: The check for GLFW_RELEASE probably relies on Windows repeat logic, probably don't want to rely on that 
//		 long-term				-carver (8-10-20)

void control_play(int action, int key) {
	if (action == GLFW_RELEASE) return;
	
	if (key == GLFW_KEY_1) {
		if (mode == MODE_PLAY_FIRST_PERSON) {
			mode = MODE_PLAY;
			addTextToBox("First Person Off", &eventTextBox);
			world.camera.initializeForGrid();
		}
		else {
			mode = MODE_PLAY_FIRST_PERSON;
			addTextToBox("First Person On", &eventTextBox);
			world.camera.initializeForPlayer();
		}
	}
	
	if (key == GLFW_KEY_W) {
		moveTheOther();
		if (mode == MODE_PLAY_FIRST_PERSON) {
			movePlayerForward();
		}
		else {
			movePlayer(UP);
		}
	}

	if (key == GLFW_KEY_A) {
		if (mode == MODE_PLAY_FIRST_PERSON) {
			rotatePlayer(LEFT);
		}
		else {
			moveTheOther();
			movePlayer(LEFT);
		}
	}
	
	if (key == GLFW_KEY_S) {
		moveTheOther();

		if (mode == MODE_PLAY_FIRST_PERSON) {
			movePlayerBackward();
		}
		else {
			movePlayer(DOWN);
		}
	}

	if (key == GLFW_KEY_D) {
		if (mode == MODE_PLAY_FIRST_PERSON) {
			rotatePlayer(RIGHT);
		}
		else {
			moveTheOther();
			movePlayer(RIGHT);
		}
	}

	if (key == GLFW_KEY_SPACE) {
		moveTheOther();
	}

	if (key == GLFW_KEY_ENTER) {
		attack();
	}

	if (key == GLFW_KEY_L) {
		if (world.theOther.actionState == ACTION_STATE_AVOIDANT) {
			addTextToBox("AI Set To SEEKING", &eventTextBox);
			world.theOther.actionState = ACTION_STATE_SEEKING;
		}
		else if (world.theOther.actionState == ACTION_STATE_SEEKING) {
			addTextToBox("AI Set To AVOIDANT", &eventTextBox);
			world.theOther.actionState = ACTION_STATE_AVOIDANT;
		}
	}
}

void control_freeCam(int action, int key) {
	if (action == GLFW_RELEASE) return;

	if (key == GLFW_KEY_L) {
		wallModel.scale(1.1f);
	}
	
	if (key == GLFW_KEY_M) {
		wallModel.scale(0.89f);
	}

	if (key == GLFW_KEY_G) {
		guidingGrid = !guidingGrid;
		addTextToBox("Guiding Grid: " + std::to_string(guidingGrid), &eventTextBox);
	}
	
	if (key == GLFW_KEY_R) {
		// Grid goes bye-bye when this happens.
		regenerateMap();
	}
	
	if (key == GLFW_KEY_O) {
		lightOrbit = !lightOrbit;
		addTextToBox("Light Orbit: " + std::to_string(lightOrbit), &eventTextBox);
	}
}

void control_edit(int action, int key) {
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
