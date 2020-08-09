/*
	Random TODOs:
	- make player and enemies smaller than the grid space
		- this then allows for simple attack animations toward the defender
		- make the player or enemies non-cubeish?
	- make more than one enemy
	- consolidate key mappings <-- THIS IS A MESS!!!
	- make some sense with all the directions being thrown around
		- directions for movement of camera
		- directions for movement along the "play grid"
		- probably others I'm not thinking of right now
	- draw entire world grid?
	- minimap
	- figure out how to metaprogram in C/C++ (see: jon blow's console commands created in jai)
	- allow for holding down directions to move player
	- set the views for shaders somewhere else (currently directly placed in main loop) -- probably not a big deal now (with only 3 active shaders)
	- figure out why regenerateMap breaks all the things (probably something stupid)	
	- keep consistent viewport ratio when resizing window
	- frame timing
	- create movement for player and theOther -- don't just jump over (could add move buffer here, too)
		-- same with switching between 3rd and 1st person -- slowly zoom in and rotate camera (will help you get a better hold on camera stuffs)
	- probably want to get away from just using headers -- the ordering of includes is getting to be a pain (also, this'll help you bring global state back into main + keep it there/pass references)
*/

#include <glad/glad.h>
#include <glfw3.h>

#include <time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "constants.h"

glm::vec3 getPlayerModelCoords();

struct Light {

	glm::vec3 pos;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	float currentDegrees = 0;

};

unsigned int currentScreenHeight	= INITIAL_SCREEN_HEIGHT;
unsigned int currentScreenWidth		= INITIAL_SCREEN_WIDTH;

#include "model.h"
#include "camera.h"

#include "worldState.h"

#include "shader.h"
#include "worldGeneration.h"
#include "textBox.h"

unsigned int w_prevState = GLFW_RELEASE;
unsigned int a_prevState = GLFW_RELEASE;
unsigned int s_prevState = GLFW_RELEASE;
unsigned int d_prevState = GLFW_RELEASE;
unsigned int c_prevState = GLFW_RELEASE;
unsigned int l_prevState = GLFW_RELEASE;
unsigned int m_prevState = GLFW_RELEASE;
unsigned int g_prevState = GLFW_RELEASE;
unsigned int r_prevState = GLFW_RELEASE;
unsigned int o_prevState = GLFW_RELEASE;
unsigned int f_prevState = GLFW_RELEASE;
unsigned int p_prevState = GLFW_RELEASE;
unsigned int one_prevState = GLFW_RELEASE;
unsigned int spacebar_prevState = GLFW_RELEASE;
unsigned int enter_prevState = GLFW_RELEASE;
unsigned int graveAccent_prevState = GLFW_RELEASE;

#define MODE_PLAY				0
#define MODE_FREE_CAMERA		1
#define MODE_LEVEL_EDIT			2
#define MODE_PLAY_FIRST_PERSON  3

unsigned int mode = MODE_PLAY;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

bool firstMouse = true;
bool freeCamera = false;

Model background;
Model theOtherModel;

Model floorModel;
Model lightCube;

Model wallModel;
Model decorativeWallModel;
Model wallCoverModel;

unsigned int regularShaderProgramID;
unsigned int lightShaderProgramID;
unsigned int UIShaderProgramID;

Font ariel;

glm::mat4 projection;

bool moveLight = false;
bool guidingGrid = false;

void moveLightAroundOrbit(float deltaTime);

#include "console.h"

Console console;


void resetProjectionMatrices() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

	glUseProgram(regularShaderProgramID);
	unsigned int projectionLoc = glGetUniformLocation(regularShaderProgramID, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glUseProgram(lightShaderProgramID);
	unsigned int lightProjectionLoc = glGetUniformLocation(lightShaderProgramID, "projection");
	glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// TODO: This is only going to get weirder when you have more fonts/shader floating around. Maybe wrap all the shaders in an 
	//		 object, then hold a list of references to them. That way you can just flip through them on updates like this.
	//														- carver (8-5-20)
	glUseProgram(ariel.shaderProgramID);
	glm::mat4 textProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	unsigned int textProjectionLoc = glGetUniformLocation(ariel.shaderProgramID, "projection");
	glUniformMatrix4fv(textProjectionLoc, 1, GL_FALSE, glm::value_ptr(textProjection));

	glUseProgram(UIShaderProgramID);
	textProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	unsigned int UIProjectionLoc = glGetUniformLocation(UIShaderProgramID, "projection");
	glUniformMatrix4fv(UIProjectionLoc, 1, GL_FALSE, glm::value_ptr(textProjection));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	currentScreenWidth	= width;
	currentScreenHeight = height;

	resetProjectionMatrices();
}

bool isTheOtherHere(int worldX, int worldY, int gridX, int gridY) {
	return	world.theOther.worldCoordX == worldX &&
			world.theOther.worldCoordY == worldY &&
			world.theOther.gridCoordX  == gridX  &&
			world.theOther.gridCoordY  == gridY;
}

bool isMapSpaceEmpty(int worldX, int worldY, int gridX, int gridY) {
	return world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[gridY][gridX] == 0;
}

void movePlayer(int direction) {
	if(mode != MODE_PLAY_FIRST_PERSON) world.player.directionFacing = direction;

	if (direction == UP) {
		int prospectiveYCoord = world.player.gridCoordY - PLAYER_SPEED;
		if (prospectiveYCoord >= 0 && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[prospectiveYCoord][world.player.gridCoordX] == 0 &&
			!isTheOtherHere(world.player.worldCoordX, world.player.worldCoordY, world.player.gridCoordX, prospectiveYCoord)) {
			world.player.gridCoordY = prospectiveYCoord;
		}
		else if (prospectiveYCoord < 0) {
			world.player.worldCoordY++;
			world.player.gridCoordY = GRID_MAP_SIZE_X - PLAYER_SPEED;
		}
	}

	if (direction == DOWN) {
		int prospectiveYCoord = world.player.gridCoordY + PLAYER_SPEED;
		if (prospectiveYCoord < GRID_MAP_SIZE_Y && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[prospectiveYCoord][world.player.gridCoordX] == 0
			&& !isTheOtherHere(world.player.worldCoordX, world.player.worldCoordY, world.player.gridCoordX, prospectiveYCoord)) {
			world.player.gridCoordY = prospectiveYCoord;			
		}
		else if (prospectiveYCoord == GRID_MAP_SIZE_X) {
			world.player.worldCoordY--;
			world.player.gridCoordY = 0;
		}
	}

	if (direction == LEFT) {
		int prospectiveXCoord = world.player.gridCoordX - PLAYER_SPEED;
		if (prospectiveXCoord >= 0 && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[world.player.gridCoordY][prospectiveXCoord] == 0 &&
			!isTheOtherHere(world.player.worldCoordX, world.player.worldCoordY, prospectiveXCoord, world.player.gridCoordY)) {
			world.player.gridCoordX = prospectiveXCoord;
		}
		else if (prospectiveXCoord < 0) {
			world.player.worldCoordX--;
			world.player.gridCoordX = GRID_MAP_SIZE_Y - PLAYER_SPEED;
		}
	}

	if (direction == RIGHT) {
		int prospectiveXCoord = world.player.gridCoordX + PLAYER_SPEED;
		if (prospectiveXCoord < GRID_MAP_SIZE_X && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[world.player.gridCoordY][prospectiveXCoord] == 0 &&
			!isTheOtherHere(world.player.worldCoordX, world.player.worldCoordY, prospectiveXCoord, world.player.gridCoordY)) {
			world.player.gridCoordX = prospectiveXCoord;
		}
		else if (prospectiveXCoord == GRID_MAP_SIZE_Y) {
			world.player.worldCoordX++;
			world.player.gridCoordX = 0;
		}
	}

	if(mode == MODE_PLAY_FIRST_PERSON) world.camera.position = getPlayerModelCoords();
}

void movePlayerForward() {
	switch (world.player.directionFacing) {
		case UP:
			movePlayer(UP);
			break;
		case DOWN:
			movePlayer(DOWN);
			break;
		case LEFT:
			movePlayer(LEFT);
			break;
		case RIGHT:
			movePlayer(RIGHT);
			break;
	}
}

void movePlayerBackward() {
	switch (world.player.directionFacing) {
		case UP:
			movePlayer(DOWN);
			break;
		case DOWN:
			movePlayer(UP);
			break;
		case LEFT:
			movePlayer(RIGHT);
			break;
		case RIGHT:
			movePlayer(LEFT);
			break;	
	}
}

void rotatePlayer(int direction) {
	if (direction == LEFT) {
		if (world.player.directionFacing == UP)		world.player.directionFacing = LEFT;
		else if (world.player.directionFacing == LEFT)	world.player.directionFacing = DOWN;
		else if (world.player.directionFacing == DOWN)	world.player.directionFacing = RIGHT;
		else if (world.player.directionFacing == RIGHT)	world.player.directionFacing = UP;
	}

	if (direction == RIGHT) {
		if (world.player.directionFacing == UP)		world.player.directionFacing = RIGHT;
		else if (world.player.directionFacing == RIGHT)	world.player.directionFacing = DOWN;
		else if (world.player.directionFacing == DOWN)	world.player.directionFacing = LEFT;
		else if (world.player.directionFacing == LEFT)	world.player.directionFacing = UP;
	}
}

void attack() {
	int xAttack = world.player.gridCoordX;
	int yAttack = world.player.gridCoordY;

	switch (world.player.directionFacing) {
		case (UP):
			yAttack--;
			break;
		case (DOWN):
			yAttack++;
			break;
		case (LEFT):
			xAttack--;
			break;
		case (RIGHT):
			xAttack++;
			break;
	}

	if (isTheOtherHere(world.player.worldCoordX, world.player.worldCoordY, xAttack, yAttack)) {
		world.theOther.hitPoints -= world.player.strength;
		if (world.theOther.hitPoints <= 0) {
			world.theOther.worldCoordX = 20;
			world.theOther.worldCoordY = 20;
		}
	}
}

void moveTheOther() {
	if (world.theOther.worldCoordX != world.player.worldCoordX ||
		world.theOther.worldCoordY != world.player.worldCoordY) return;

	int diffToPlayerX = world.theOther.gridCoordX - world.player.gridCoordX;
	int diffToPlayerY = world.theOther.gridCoordY - world.player.gridCoordY;

	int prospectiveGridCoordX = world.theOther.gridCoordX;
	int prospectiveGridCoordY = world.theOther.gridCoordY;

	if (world.theOther.actionState == ACTION_STATE_SEEKING) {
		
		if (diffToPlayerX == 0 && diffToPlayerY == 0) return;
		if (diffToPlayerX == 0 && abs(diffToPlayerY) == 1) return;
		if (abs(diffToPlayerX) == 1 && diffToPlayerY == 0) return;

		if (rand() % 2 == 0 && diffToPlayerX != 0) {
			if (diffToPlayerX > 0)		prospectiveGridCoordX = world.theOther.gridCoordX - 1;
			else if (diffToPlayerX < 0) prospectiveGridCoordX = world.theOther.gridCoordX + 1;
		}
		else {
			if (diffToPlayerY == 0) {
				if (diffToPlayerX > 0)		prospectiveGridCoordX = world.theOther.gridCoordX - 1;
				else if (diffToPlayerX < 0) prospectiveGridCoordX = world.theOther.gridCoordX + 1;
			}

			if (diffToPlayerY > 0)		prospectiveGridCoordY = world.theOther.gridCoordY - 1;
			else if (diffToPlayerY < 0) prospectiveGridCoordY = world.theOther.gridCoordY + 1;
		}
	}
	// TODO: Finish this. The Other can go off map when avoidant.
	else if (world.theOther.actionState == ACTION_STATE_AVOIDANT) {
		if (diffToPlayerX > diffToPlayerY) {
			if		(diffToPlayerX > 0)	prospectiveGridCoordX = world.theOther.gridCoordX + 1;
			else if (diffToPlayerX < 0) prospectiveGridCoordX = world.theOther.gridCoordX - 1;

			
		}	else {
			if (diffToPlayerY > 0)		prospectiveGridCoordY = world.theOther.gridCoordY + 1;
			else if (diffToPlayerY < 0) prospectiveGridCoordY = world.theOther.gridCoordY - 1;
		}
	}

	if (isMapSpaceEmpty(world.player.worldCoordX, world.player.worldCoordY, prospectiveGridCoordX, prospectiveGridCoordY)) {
		world.theOther.gridCoordX = prospectiveGridCoordX;
		world.theOther.gridCoordY = prospectiveGridCoordY;
	}
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (console.consoleOut) {
		console.addInput(codepoint);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS) return;
	
	if (key == GLFW_KEY_F1) {
		console.flipOut();
	}

	if (!console.consoleOut) return;
	
	if (key == GLFW_KEY_BACKSPACE) {
		console.removeCharacter();
	}

	if (key == GLFW_KEY_ENTER) {
		console.submit();
	}
}


void processKeyboardInput(GLFWwindow *window) {
	// NOTE: This strategy is not nearly robust enough. It relies on polling the keyboard events. 
	//		 Definite possibility of missing a press or release event here. And frame timing has not
	//		 yet been considered. Probably more reading is required. Still, good enough for exploratory work.
	//			-- This should probably be a callback like the mouse input... I think.

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (console.consoleOut) return;

	int c_currentState = glfwGetKey(window, GLFW_KEY_C);
	if (c_currentState == GLFW_PRESS && c_prevState == GLFW_RELEASE) {
		// If we're going to stay with multiple states like this, we probably should get away from 
		// a rotation of states, and instead go with a key binding for each state (or console command?).
		// When that's the case, it'll be easier to consolidate all the global state changes that each
		// state needs for its functionality, view, etc... - Carver (8-4-20)
		
		if (mode == MODE_PLAY || mode == MODE_PLAY_FIRST_PERSON) {
			mode = MODE_FREE_CAMERA;

			world.camera.initializeForGrid();
			addTextToBox("Mode: Free Camera", &eventTextBox);
		} else if (mode == MODE_FREE_CAMERA) {
			world.camera.initializeOverhead();

			mode = MODE_LEVEL_EDIT;
			addTextToBox("Mode: Level Edit", &eventTextBox);
		} else if (mode == MODE_LEVEL_EDIT) {
			world.camera.initializeForGrid();
			
			mode = MODE_PLAY;
			addTextToBox("Mode: Play", &eventTextBox);
		}
	}
	c_prevState = c_currentState;

	if (mode == MODE_FREE_CAMERA) {
		const float cameraSpeed = 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			world.camera.moveForward(deltaTime);		
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			world.camera.moveBack(deltaTime);
		}
		// strafe movement
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			world.camera.moveLeft(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			world.camera.moveRight(deltaTime);
		}

		int l_currentState = glfwGetKey(window, GLFW_KEY_L);
		if (l_currentState == GLFW_PRESS && l_prevState == GLFW_RELEASE) {
			wallModel.scale(1.1f);
		}
		l_prevState = l_currentState;

		int m_currentState = glfwGetKey(window, GLFW_KEY_M);
		if (m_currentState == GLFW_PRESS && m_prevState == GLFW_RELEASE) {
			wallModel.scale(0.89f);
		}
		m_prevState = m_currentState;

		int g_currentState = glfwGetKey(window, GLFW_KEY_G);
		if (g_currentState == GLFW_PRESS && g_prevState == GLFW_RELEASE) {
			guidingGrid = !guidingGrid;
			addTextToBox("Guiding Grid: " + std::to_string(guidingGrid), &eventTextBox);
		}
		g_prevState = g_currentState;

		int r_currentState = glfwGetKey(window, GLFW_KEY_R);
		if (r_currentState == GLFW_PRESS && r_prevState == GLFW_RELEASE) {
			// WARNING: Grid goes bye-bye when this happens.
			regenerateMap();
		}
		r_prevState = r_currentState;

		int o_currentState = glfwGetKey(window, GLFW_KEY_O);
		if (o_currentState == GLFW_PRESS && o_prevState == GLFW_RELEASE) {
			moveLight = !moveLight;
			addTextToBox("Light Orbit: " + std::to_string(moveLight), &eventTextBox);
		}
		o_prevState = o_currentState;
	}
	else if (mode == MODE_PLAY || mode == MODE_PLAY_FIRST_PERSON) {
		int one_currentState = glfwGetKey(window, GLFW_KEY_1);
		if (one_currentState == GLFW_PRESS && one_prevState == GLFW_RELEASE) {
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
		one_prevState = one_currentState;

		int w_currentState = glfwGetKey(window, GLFW_KEY_W);
		if (w_currentState == GLFW_PRESS && w_prevState == GLFW_RELEASE) {
			moveTheOther();
			if (mode == MODE_PLAY_FIRST_PERSON) {
				movePlayerForward();
			} else { 
				movePlayer(UP); 
			}
		}
		w_prevState = w_currentState;

		int a_currentState = glfwGetKey(window, GLFW_KEY_A);
		if (a_currentState == GLFW_PRESS && a_prevState == GLFW_RELEASE) {
			if (mode == MODE_PLAY_FIRST_PERSON) {
				rotatePlayer(LEFT);
			} else {
				moveTheOther();
				movePlayer(LEFT);
			}
		}
		a_prevState = a_currentState;

		int s_currentState = glfwGetKey(window, GLFW_KEY_S);
		if (s_currentState == GLFW_PRESS && s_prevState == GLFW_RELEASE) {
			moveTheOther();

			if (mode == MODE_PLAY_FIRST_PERSON) {
				movePlayerBackward();
			} else {
				movePlayer(DOWN);
			}
		}
		s_prevState = s_currentState;

		int d_currentState = glfwGetKey(window, GLFW_KEY_D);
		if (d_currentState == GLFW_PRESS && d_prevState == GLFW_RELEASE) {
			if (mode == MODE_PLAY_FIRST_PERSON) {
				rotatePlayer(RIGHT);
			} else {
				moveTheOther();
				movePlayer(RIGHT);
			}	         
		}
		d_prevState = d_currentState;

		int spacebar_currentState = glfwGetKey(window, GLFW_KEY_SPACE);
		if (spacebar_currentState == GLFW_PRESS && spacebar_prevState == GLFW_RELEASE) {
			moveTheOther();
		}
		spacebar_prevState = spacebar_currentState;

		int enter_currentState = glfwGetKey(window, GLFW_KEY_ENTER);
		if (enter_currentState == GLFW_PRESS && enter_prevState == GLFW_RELEASE) {
			attack();
		}
		enter_prevState = enter_currentState;

		int l_currentState = glfwGetKey(window, GLFW_KEY_L);
		if (l_currentState == GLFW_PRESS && l_prevState == GLFW_RELEASE) {
			if (world.theOther.actionState == ACTION_STATE_AVOIDANT) {
				addTextToBox("AI Set To AVOIDANT", &eventTextBox);
				world.theOther.actionState = ACTION_STATE_SEEKING;
			}
			else if (world.theOther.actionState == ACTION_STATE_SEEKING) {
				addTextToBox("AI Set To SEEKING", &eventTextBox);
				world.theOther.actionState = ACTION_STATE_AVOIDANT;
			}
		}
		l_prevState = l_currentState;
	}	
	else if (mode == MODE_LEVEL_EDIT) {
		const float cameraSpeed = 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			world.camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			world.camera.moveBack(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			world.camera.moveLeft(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			world.camera.moveRight(deltaTime);
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (mode != MODE_LEVEL_EDIT) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (lastCursorX < 0 || lastCursorX > currentScreenWidth ||
			lastCursorY < 0 || lastCursorY > currentScreenHeight) {
			addTextToBox("Cursor is off screen, &eventTextBox", &eventTextBox);
		}
		else {
			addTextToBox("LastCursor: (" + std::to_string(lastCursorX) + ", " + std::to_string(lastCursorY) + ")", &eventTextBox);
			glm::vec4 clickCoords = glm::vec4(lastCursorX, lastCursorY, 1.0f, 1.0f); // shouldn't this be a vec2???
			glm::vec4 viewCoords  = clickCoords * glm::inverse(projection);
			//glm::vec4 worldCoords = viewCoords * glm::inverse(world.camera.generateView());

			glm::vec4 worldCoords = glm::inverse(world.camera.generateView()) * glm::inverse(projection) * clickCoords;

			addTextToBox("ClickCoords: (" + std::to_string(clickCoords.x) + ", " + std::to_string(clickCoords.y) + ", " + std::to_string(clickCoords.z) + ", " + std::to_string(clickCoords.w) + ")", &eventTextBox);
			addTextToBox("ViewCoords: (" + std::to_string(viewCoords.x) + ", " + std::to_string(viewCoords.y) + ", " + std::to_string(viewCoords.z) + ", " + std::to_string(viewCoords.w) + ")", &eventTextBox);
			addTextToBox("WorldCoords: (" + std::to_string(worldCoords.x) + ", " + std::to_string(worldCoords.y) + ", " + std::to_string(worldCoords.z) + ", " + std::to_string(worldCoords.w) + ")", &eventTextBox);
		}
	}
}

void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	// TODO: Figure out how to lockdown the cursor such that the coordinate args can't go outside the viewport
	//		 Clamping won't work.
	
	if (mode == MODE_FREE_CAMERA) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (firstMouse)
		{
			lastCursorX = (float)xPos;
			lastCursorY = (float)yPos;
			firstMouse = false;
		}

		float xOffset = (float)(xPos - lastCursorX);
		float yOffset = (float)(lastCursorY - yPos);

		world.camera.adjustYawAndPitch(xOffset, yOffset);		
	}
	else if (mode == MODE_LEVEL_EDIT) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstMouse = true;

	} else if (mode == MODE_PLAY) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstMouse = true;
	}

	lastCursorX = (float)xPos;
	lastCursorY = (float)yPos;
}

void createLightCube() {
	
	float vertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 0.5f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.5f,	0.0f, 0.0f, 1.0f,

		// bottom
		// 5, 6, 7, 8
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// left
		// 1, 4, 5, 8
		0.0f, 0.5f, 0.5f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// right
		// 2, 3, 6, 7
		0.5f, 0.5f, 0.5f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.5f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 0.5f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,	0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

		// back		
		// 1, 2, 5, 6
		0.0f, 0.5f, 0.5f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.5f,	0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f,
	};

	unsigned int indices[] = {
		// top
		0, 1, 3,
		1, 2, 3,

		// bottom
		4, 5, 7,
		5, 6, 7,

		// left
		0, 3, 4,
		3, 4, 7,

		// right
		1, 2, 5,
		2, 5, 6,

		// front
		3, 2, 7,
		2, 7, 6,

		// back
		0, 1, 4,
		1, 4, 5
	};

	Mesh lightMesh;
	
	for (int i = 0; i < sizeof(vertices) / sizeof(float); i++) {
		lightMesh.vertices.push_back(vertices[i]);
	}

	for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++) {
		lightMesh.indices.push_back(indices[i]);		
	}

	lightMesh.setupVAO();
	lightMesh.shaderProgramID = lightShaderProgramID;
	glm::vec3 color1 = glm::vec3(1.0f, 1.0f, 1.0f);
	lightMesh.material.diffuse.x = color1.x;
	lightMesh.material.diffuse.y = color1.y;
	lightMesh.material.diffuse.z = color1.z;	

	lightCube.meshes.push_back(lightMesh);
	lightCube.worldOffset = glm::vec3(-2.0f, -5.0f, 4.0f);
}

void createGridFloorAndWallModels() {
	// NOTE: These coords are in local space
	// TODO: add materials??
	
	float floorVertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 0.5f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.5f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.5f,	0.0f, 0.0f, 1.0f,
		
		// bottom
		// 5, 6, 7, 8
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// left
		// 1, 4, 5, 8
		0.0f, 0.5f, 0.5f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// right
		// 2, 3, 6, 7
		0.5f, 0.5f, 0.5f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.5f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 0.5f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,	0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

		// back		
		// 1, 2, 5, 6
		0.0f, 0.5f, 0.5f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.5f,	0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f,
	};

	float wallVertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 0.5f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		
		// bottom
		// 5, 6, 7, 8
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// left
		// 1, 4, 5, 8
		0.0f, 0.5f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// right
		// 2, 3, 6, 7
		0.5f, 0.5f, 1.0f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

		// back		
		// 1, 2, 5, 6
		0.0f, 0.5f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f	
	};

	unsigned int indices[] = {
		// top
		0, 1, 3,
		1, 2, 3,

		// bottom
		4, 5, 7,
		5, 6, 7,

		// left
		8, 9, 10,
		9, 10, 11,

		// right
		12, 13, 14,
		13, 14, 15,

		// front
		16, 17, 18,
		17, 18, 19,

		// back
		20, 21, 22,
		21, 22, 23
	};
	
	Mesh floorMesh;	
	Mesh wallMesh;

	for (int i = 0; i < sizeof(floorVertices) / sizeof(float); i++) {
		floorMesh.vertices.push_back(floorVertices[i]);
		wallMesh.vertices.push_back(wallVertices[i]);
	}

	for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++) {
		floorMesh.indices.push_back(indices[i]);
		wallMesh.indices.push_back(indices[i]);
	}
	
	floorMesh.setupVAO();
	floorMesh.shaderProgramID = regularShaderProgramID;
	glm::vec3 color1 = glm::vec3(0.4f, 1.0f, 1.0f);
	floorMesh.material.diffuse.x = color1.x;
	floorMesh.material.diffuse.y = color1.y;
	floorMesh.material.diffuse.z = color1.z;
	floorMesh.material.ambient.x = color1.x;
	floorMesh.material.ambient.y = color1.y;
	floorMesh.material.ambient.z = color1.z;

	wallMesh.setupVAO();
	wallMesh.shaderProgramID = regularShaderProgramID;
	glm::vec3 color2 = glm::vec3(1.0f, 0.5f, 0.5f);
	wallMesh.material.diffuse.x = color2.x;
	wallMesh.material.diffuse.y = color2.y;
	wallMesh.material.diffuse.z = color2.z;
	wallMesh.material.ambient.x = color2.x;
	wallMesh.material.ambient.y = color2.y;
	wallMesh.material.ambient.z = color2.z;

	floorModel.meshes.push_back(floorMesh);
	wallModel.meshes.push_back(wallMesh);
}

void createPlayerAndTheOtherModels() {
	// NOTE: These coords are in local space
	// TODO: add materials??

	float playerVertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 0.5f, 0.25f,	0.0f, 0.0f, 1.0f, // 0
		0.5f, 0.5f, 0.25f,	0.0f, 0.0f, 1.0f, // 1
		0.5f, 0.0f, 0.25f,	0.0f, 0.0f, 1.0f, // 2
		0.0f, 0.0f, 0.25f,	0.0f, 0.0f, 1.0f, // 3

		// bottom
		// 5, 6, 7, 8
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f, // 4
		0.5f, 0.5f, 0.0f,	0.0f, 0.0f, -1.0f, // 5
		0.5f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f, // 6
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f, // 7

		// left
		// 1, 4, 5, 8
		0.0f, 0.5f, 0.25f,	-1.0f, 0.0f, 0.0f, // 8
		0.0f, 0.0f, 0.25f,	-1.0f, 0.0f, 0.0f, // 9
		0.0f, 0.5f, 0.0f,	-1.0f, 0.0f, 0.0f, // 10
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f, // 11

		// right
		// 2, 3, 6, 7
		0.5f, 0.5f, 0.25f,	1.0f, 0.0f, 0.0f, // 12
		0.5f, 0.0f, 0.25f,	1.0f, 0.0f, 0.0f, // 13
		0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f, // 14
		0.5f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f, // 15

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 0.25f,	0.0f, 1.0f, 0.0f, // 16
		0.5f, 0.0f, 0.25f,	0.0f, 1.0f, 0.0f, // 17
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f, // 18
		0.5f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f, // 19

		// back		
		// 1, 2, 5, 6
		0.0f, 0.5f, 0.25f,	0.0f, -1.0f, 0.0f, // 20
		0.5f, 0.5f, 0.25f,	0.0f, -1.0f, 0.0f, // 21
		0.0f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f, // 22
		0.5f, 0.5f, 0.0f,	0.0f, -1.0f, 0.0f, // 23
	};

	unsigned int indices[] = {
		// top
		0, 1, 3,
		1, 2, 3,

		// bottom
		4, 5, 7,
		5, 6, 7,

		// left
		8, 9, 10,
		9, 10, 11,

		// right
		12, 13, 14,
		13, 14, 15,

		// front
		16, 17, 18,
		17, 18, 19,

		// back
		20, 21, 22,
		21, 22, 23
	};

	Mesh playerMesh;
	Mesh theOtherMesh;

	for (int i = 0; i < sizeof(playerVertices) / sizeof(float); i++) {
		playerMesh.vertices.push_back(playerVertices[i]);
	}

	for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++) {
		playerMesh.indices.push_back(indices[i]);
	}

	playerMesh.setupVAO();
	playerMesh.shaderProgramID = regularShaderProgramID;
	glm::vec3 color = glm::vec3(0.0f, 0.52f, 0.9f);
	playerMesh.material.diffuse.r = color.x;
	playerMesh.material.diffuse.g = color.y;
	playerMesh.material.diffuse.b = color.z;
	playerMesh.material.ambient.r = color.x;
	playerMesh.material.ambient.g = color.y;
	playerMesh.material.ambient.b = color.z;

	background.meshes.push_back(playerMesh);

	for (int i = 0; i < sizeof(playerVertices) / sizeof(float); i++) {
		theOtherMesh.vertices.push_back(playerVertices[i]);
	}

	for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++) {
		theOtherMesh.indices.push_back(indices[i]);
	}

	theOtherMesh.setupVAO();
	theOtherMesh.shaderProgramID = regularShaderProgramID;
	glm::vec3 color2 = glm::vec3(1.0f, 1.00f, 0.5f); 
	theOtherMesh.material.diffuse.r = color2.x;
	theOtherMesh.material.diffuse.g = color2.y;
	theOtherMesh.material.diffuse.b = color2.z;
	theOtherMesh.material.ambient.r = color2.x;
	theOtherMesh.material.ambient.g = color2.y;
	theOtherMesh.material.ambient.b = color2.z;


	theOtherModel.meshes.push_back(theOtherMesh);
}

void drawGrid() {

	float zOffset = 0.0f;
	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		float yOffset = -0.5f * row;

		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			float xOffset = 0.5f * column;
			glm::vec3 offset = glm::vec3(xOffset, yOffset, zOffset);

			if (world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[row][column] != 0) {
				wallModel.worldOffset = offset;
				wallModel.draw();
			} else {
				floorModel.worldOffset = offset;
				floorModel.draw();
			}
		}
	}
}

glm::vec3 getPlayerModelCoords() {
	float zOffset = 0.5f;
	if (mode == MODE_PLAY_FIRST_PERSON) zOffset += 0.25f;
	
	float yOffset = -0.5f * world.player.gridCoordY;
	float xOffset = 0.5f * world.player.gridCoordX;
	
	return glm::vec3(xOffset, yOffset, zOffset);
}

void drawPlayer() {
	background.worldOffset = getPlayerModelCoords();
	background.draw();
}

void drawTheOther() {
	if (world.theOther.worldCoordX != world.player.worldCoordX || world.theOther.worldCoordY != world.player.worldCoordY) return;
	
	float zOffset = 0.5f;
	float yOffset = -0.5f * world.theOther.gridCoordY;
	float xOffset = 0.5f * world.theOther.gridCoordX;

	theOtherModel.worldOffset = glm::vec3(xOffset, yOffset, zOffset);
	theOtherModel.draw();
}

void moveLightAroundOrbit(float deltaTime) {
	float radius = 5.0f;
	float speed = 90.0f; // degrees / second
	float degreesMoved = speed * deltaTime;

	float midGridX = 0.5f * (GRID_MAP_SIZE_X / 2);
	float midGridY = -0.5f * GRID_MAP_SIZE_Y / 2;

	float newDegrees = world.light.currentDegrees + degreesMoved;
	if (newDegrees > 360) newDegrees -= 360;

	float newX = glm::cos(glm::radians(newDegrees)) * radius + midGridX;
	float newY = glm::sin(glm::radians(newDegrees)) * radius + midGridY;

	world.light.pos.x = newX;
	world.light.pos.y = newY;
	lightCube.worldOffset.x = newX;
	lightCube.worldOffset.y = newY;

	world.light.currentDegrees = newDegrees;
}

// TODO
void drawCameraStats() {

	
	//drawText(textShaderProgramID, "butts", 200, 600, 1.9f, glm::vec3(1.0f, 1.0f, 1.0f));

}

// GRID LINES
unsigned int gridVAO_ID, gridVBO_ID;
unsigned int numGridVertices = 0;
void guidingGridSetup() {

	float lowerZ = 0.0f;
	float upperZ = 1.0f;
	float zStep	 = 1.0f;

	float lowerY = -10.f;
	float upperY =  10.f;
	float yStep  =  0.5f;

	float lowerX = -10.f;
	float upperX =	10.f;
	float xStep	 =  0.5;

	std::vector<float> vertices;

	// lines along x axis
	for (float y = lowerY; y <= upperY; y += yStep) {
		for (float z = lowerZ; z <= upperZ; z += zStep) {
			vertices.push_back(lowerX);
			vertices.push_back(y);
			vertices.push_back(z);

			vertices.push_back(upperX);
			vertices.push_back(y);
			vertices.push_back(z);

			numGridVertices += 2;
		}
	}

	// lines along y axis
	for (float x = lowerX; x <= upperX; x += xStep) {
		for (float z = lowerZ; z <= upperZ; z += zStep) {
			vertices.push_back(x);
			vertices.push_back(lowerY);
			vertices.push_back(z);

			vertices.push_back(x);
			vertices.push_back(upperY);
			vertices.push_back(z);

			numGridVertices += 2;
		}		
	}

	glGenVertexArrays(1, &gridVAO_ID);
	glBindVertexArray(gridVAO_ID);

	glGenBuffers(1, &gridVBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void doAGuidingGridThing() {
	glUseProgram(regularShaderProgramID);

	glBindVertexArray(gridVAO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO_ID);

	unsigned int modelLoc = glGetUniformLocation(regularShaderProgramID, "model");
	glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)); // TODO: this can just be the 0 mat4, right?
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));

	unsigned int objectDiffuseLoc = glGetUniformLocation(regularShaderProgramID, "objectDiffuse");
	glUniform3f(objectDiffuseLoc, 1.0f, 1.0f, 1.0f);

	unsigned int objectAmbientLoc = glGetUniformLocation(regularShaderProgramID, "objectAmbient");
	glUniform3f(objectAmbientLoc, 1.0f, 1.0f, 1.0f);

	glDrawArrays(GL_LINES, 0, numGridVertices);
}

int main() {
	// ------------ INIT STUFF -------------

	// initialization of glfw and glad libraries, window creation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE__	

	GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "GridGame1", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ------------- SHADERS -------------	
	regularShaderProgramID	= createShaderProgram("vertexShader.vert", "fragmentShader.frag");
	lightShaderProgramID	= createShaderProgram("lightVertexShader.vert", "lightFragmentShader.frag");
	UIShaderProgramID		= createShaderProgram("UIVertexShader.vert", "UIFragmentShader.frag");
	
	glUseProgram(regularShaderProgramID);
	unsigned int viewLoc = glGetUniformLocation(regularShaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(regularShaderProgramID, "projection");
	
	projection = glm::perspective(glm::radians(45.0f), (float) INITIAL_SCREEN_WIDTH / (float) INITIAL_SCREEN_HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	glUseProgram(lightShaderProgramID);
	unsigned int lightViewLoc = glGetUniformLocation(lightShaderProgramID, "view");
	unsigned int lightProjectionLoc = glGetUniformLocation(lightShaderProgramID, "projection");
	glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glUseProgram(UIShaderProgramID);
	glm::mat4 UIProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	unsigned int UIProjectionLoc = glGetUniformLocation(UIShaderProgramID, "projection");
	glUniformMatrix4fv(UIProjectionLoc, 1, GL_FALSE, glm::value_ptr(UIProjection));

	// initializing viewport and setting callback for window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCharCallback(window, character_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);

	createLightCube();
	
	world.light.pos = glm::vec3(0.0f);
	world.light.ambient = glm::vec3(1.0f);
	world.light.diffuse = glm::vec3(1.0f);
	
	createGridFloorAndWallModels();
	createPlayerAndTheOtherModels();

	// set seed and generate map
	unsigned int seed = (unsigned int)time(NULL); // seconds since Jan 1, 2000
	srand(seed);
	addTextToBox("seed: " + std::to_string(seed), &eventTextBox);
	
	generateWorldMap(&world);

	// CHARACTER INITIALIZATION
	world.player.worldCoordX = PLAYER_WORLD_START_X;
	world.player.worldCoordY = PLAYER_WORLD_START_Y;
	world.player.gridCoordX = PLAYER_GRID_START_X;
	world.player.gridCoordY = PLAYER_GRID_START_Y;
	world.player.strength = 1;
	world.player.hitPoints = 20;
	world.player.directionFacing = UP;

	world.theOther.worldCoordX = PLAYER_WORLD_START_X;
	world.theOther.worldCoordY = PLAYER_WORLD_START_Y;
	world.theOther.gridCoordX = 1;
	world.theOther.gridCoordY = 2;
	world.theOther.hitPoints = 3;
	world.theOther.strength = 1;

	lastFrameTime = (float)glfwGetTime();

	initializeFont("arial.ttf", &ariel);
	eventTextBox.font = &ariel;
	console.historyTextbox.font = &ariel;

	addTextToBox("butt1", &console.historyTextbox);
	addTextToBox("butt2", &console.historyTextbox);
	addTextToBox("butt3", &console.historyTextbox);
	addTextToBox("butt4", &console.historyTextbox);

	// need alpha blending for text transparency
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	world.camera.initializeForGrid();

	guidingGridSetup();

	console.setup();

	// game loop
	while (!glfwWindowShouldClose(window)) {
		processKeyboardInput(window);
		
		glfwPollEvents();

		if (mode == MODE_PLAY_FIRST_PERSON) world.camera.position = getPlayerModelCoords();
		
		glUseProgram(regularShaderProgramID);
		// TODO: This needs to be done somewhere else. This will break when there's more than one shader for world objects.
		//		 Per each draw invocation? Or when the view changes, put it in all the shaders? ehhh...
		glm::mat4 view;
		if (mode == MODE_PLAY_FIRST_PERSON) {
			view = world.camera.generateFirstPersonView(world.player.directionFacing);
		}
		else {
			view = world.camera.generateView();
		}
		viewLoc = glGetUniformLocation(regularShaderProgramID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glUseProgram(lightShaderProgramID);
		// TODO: This needs to be done somewhere else. This will break when there's more than one shader for world objects.
		//		 Per each draw invocation? Or when the view changes, put it in all the shaders? ehhh...
		lightViewLoc = glGetUniformLocation(lightShaderProgramID, "view");
		glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);

		glDepthFunc(GL_LESS);
		if(guidingGrid)	doAGuidingGridThing();
		if(moveLight) moveLightAroundOrbit(deltaTime);
		
		lightCube.draw();
		drawGrid();
		if(mode != MODE_PLAY_FIRST_PERSON) drawPlayer();
		drawTheOther();

		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		console.draw(deltaTime);
		drawTextBox(&eventTextBox);
		//drawCameraStats();

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}