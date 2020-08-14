/*
	Random TODOs:
	- fix first person mode after moving world offset info into entity
	- make player and enemies smaller than the grid space
		- this then allows for simple attack animations toward the defender
		- make the player or enemies non-cubeish?
	- make more than one enemy
	- make some sense with all the directions being thrown around
		- directions for movement of camera
		- directions for movement along the "play grid"
		- probably others I'm not thinking of right now
		- note: use cardinal directions for grid movement
	- draw entire world grid?
	- minimap
	- figure out how to metaprogram in C/C++ (see: jon blow's console commands created in jai)
	- figure out why regenerateMap breaks all the things (probably something stupid)	
	- keep consistent viewport ratio when resizing window
	- frame timing
	- create movement for player and enemy -- don't just jump over (could add move buffer here, too)
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

struct Light {

	glm::vec3 pos = glm::vec3(0.0f);;

	// TODO: maybe make a material???
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	float currentDegrees = 0;
};

#include "model.h"
#include "camera.h"

#include "worldState.h"

#include "shader.h"
#include "worldGeneration.h"
#include "textBox.h"

#define MODE_PLAY				0
#define MODE_FREE_CAMERA		1
#define MODE_LEVEL_EDIT			2
#define MODE_PLAY_FIRST_PERSON  3

unsigned int currentScreenHeight	= INITIAL_SCREEN_HEIGHT;
unsigned int currentScreenWidth		= INITIAL_SCREEN_WIDTH;

unsigned int regularShaderProgramID;
unsigned int lightShaderProgramID;
unsigned int UIShaderProgramID;
unsigned int textShaderProgramID;

unsigned int mode = MODE_PLAY;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

bool firstMouse = true;
bool freeCamera = false;

Textbox eventTextBox = {};

float cubeVertices[] = {
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
unsigned int cubeIndices[] = {
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

#define NUM_MODELS 6

struct Models {

	Models() {};
	~Models() {};

	union {
		Model mods[NUM_MODELS];

		struct {
			Model player;
			Model enemy;

			Model floorModel;
			Model wallModel;

			Model lightCube;

			Model guidingGrid;
		};
	};
};

Models models;

#define NUM_MATS 7

struct Materials {
	
	Materials() {};
	~Materials() {};

	union {
		Material mats[NUM_MATS];

		struct {
			Material light;
			Material emerald;
			Material chrome;
			Material silver;
			Material gold;
			Material blackRubber;
			Material ruby;
		};
	};
};

Materials materials;

glm::mat4 projection;

bool lightOrbit = false;
bool guidingGrid = false;

void moveLightAroundOrbit(float deltaTime);
void processConsoleCommand(std::string command);

#include "console.h"

Console console;

void resetProjectionMatrices() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

	setUniformMat4(regularShaderProgramID, "projection", projection);
	setUniformMat4(lightShaderProgramID, "projection", projection);

	glm::mat4 UIProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	
	setUniformMat4(textShaderProgramID, "projection", UIProjection);
	setUniformMat4(UIShaderProgramID, "projection", UIProjection);
}

Material* getMaterial(std::string name) {
	for (int i = 0; i < NUM_MATS; i++) {
		if (materials.mats[i].name == name) {
			return &materials.mats[i];
		}
	}

	return NULL;
}

Model* getModel(std::string name) {
	for (int i = 0; i < NUM_MODELS; i++) {
		if (models.mods[i].name == name) {
			return &models.mods[i];
		}
	}

	return NULL;
}

void setMaterial(std::string modelName, std::string matName) {
	Material *mat = getMaterial(matName);
	Model *model = getModel(modelName);
	bool fail = false;

	if (mat == NULL) {
		addTextToBox("Material not found: " + matName, &console.historyTextbox);
		fail = true;
	}
	if (model == NULL) {
		addTextToBox("Model not found: " + modelName, &console.historyTextbox);
		fail = true;
	}
	if (fail) return;

	for (int i = 0; i < model->meshes.size(); i++) {
		model->meshes[i].material = mat;
	}
	
}

void refreshView() {
	// TODO: create union with all shaders, then can reference by name, but also iterate through in cases like this
	//					and projection matrix setting -- similar thing for fonts? models? materials?
	glm::mat4 view;
	if (mode == MODE_PLAY_FIRST_PERSON) {
		view = world.camera.generateFirstPersonView(world.player.directionFacing);
	}
	else {
		view = world.camera.generateView();
	}

	setUniformMat4(regularShaderProgramID, "view", view);
	setUniformMat4(lightShaderProgramID, "view", view);

	setUniform3f(regularShaderProgramID, "viewPos", world.camera.position);
	setUniform3f(lightShaderProgramID, "viewPos", world.camera.position);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	currentScreenWidth	= width;
	currentScreenHeight = height;

	resetProjectionMatrices();
}

bool isEnemyHere(int worldX, int worldY, int gridX, int gridY) {
	return	world.enemy.worldCoordX == worldX &&
			world.enemy.worldCoordY == worldY &&
			world.enemy.gridCoords.x  == gridX  &&
			world.enemy.gridCoords.y  == gridY;
}

bool isMapSpaceEmpty(int worldX, int worldY, int gridX, int gridY) {
	return world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[gridY][gridX] == 0;
}

void movePlayer(int direction) {
	if(mode != MODE_PLAY_FIRST_PERSON) world.player.directionFacing = direction;

	if (direction == UP) {
		int prospectiveYCoord = world.player.gridCoords.y - PLAYER_SPEED;
		if (prospectiveYCoord >= 0 && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[prospectiveYCoord][world.player.gridCoords.x] == 0 &&
			!isEnemyHere(world.player.worldCoordX, world.player.worldCoordY, world.player.gridCoords.x, prospectiveYCoord)) {
			world.player.gridCoords.y = prospectiveYCoord;
		}
		else if (prospectiveYCoord < 0) {
			world.player.worldCoordY++;
			world.player.gridCoords.y = GRID_MAP_SIZE_X - PLAYER_SPEED;
		}
	}

	if (direction == DOWN) {
		int prospectiveYCoord = world.player.gridCoords.y + PLAYER_SPEED;
		if (prospectiveYCoord < GRID_MAP_SIZE_Y && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[prospectiveYCoord][world.player.gridCoords.x] == 0
			&& !isEnemyHere(world.player.worldCoordX, world.player.worldCoordY, world.player.gridCoords.x, prospectiveYCoord)) {
			world.player.gridCoords.y = prospectiveYCoord;
		}
		else if (prospectiveYCoord == GRID_MAP_SIZE_X) {
			world.player.worldCoordY--;
			world.player.gridCoords.y = 0;
		}
	}

	if (direction == LEFT) {
		int prospectiveXCoord = world.player.gridCoords.x - PLAYER_SPEED;
		if (prospectiveXCoord >= 0 && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[world.player.gridCoords.y][prospectiveXCoord] == 0 &&
			!isEnemyHere(world.player.worldCoordX, world.player.worldCoordY, prospectiveXCoord, world.player.gridCoords.y)) {
			world.player.gridCoords.x = prospectiveXCoord;
		}
		else if (prospectiveXCoord < 0) {
			world.player.worldCoordX--;
			world.player.gridCoords.x = GRID_MAP_SIZE_Y - PLAYER_SPEED;
		}
	}

	if (direction == RIGHT) {
		int prospectiveXCoord = world.player.gridCoords.x + PLAYER_SPEED;
		if (prospectiveXCoord < GRID_MAP_SIZE_X && world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[world.player.gridCoords.y][prospectiveXCoord] == 0 &&
			!isEnemyHere(world.player.worldCoordX, world.player.worldCoordY, prospectiveXCoord, world.player.gridCoords.y)) {
			world.player.gridCoords.x = prospectiveXCoord;
		}
		else if (prospectiveXCoord == GRID_MAP_SIZE_Y) {
			world.player.worldCoordX++;
			world.player.gridCoords.x = 0;
		}
	}

	//if(mode == MODE_PLAY_FIRST_PERSON) world.camera.position = getPlayerModelCoords();
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
	int xAttack = world.player.gridCoords.x;
	int yAttack = world.player.gridCoords.y;

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

	if (isEnemyHere(world.player.worldCoordX, world.player.worldCoordY, xAttack, yAttack)) {
		world.enemy.hitPoints -= world.player.strength;
		if (world.enemy.hitPoints <= 0) {
			world.enemy.worldCoordX = 20;
			world.enemy.worldCoordY = 20;
		}
	}
}

void moveEnemy() {
	if (world.enemy.worldCoordX != world.player.worldCoordX ||
		world.enemy.worldCoordY != world.player.worldCoordY) return;

	int diffToPlayerX = world.enemy.gridCoords.x - world.player.gridCoords.x;
	int diffToPlayerY = world.enemy.gridCoords.y - world.player.gridCoords.y;

	int prospectiveGridCoordX = world.enemy.gridCoords.x;
	int prospectiveGridCoordY = world.enemy.gridCoords.y;

	if (world.enemy.actionState == ACTION_STATE_SEEKING) {
		
		if (diffToPlayerX == 0 && diffToPlayerY == 0) return;
		if (diffToPlayerX == 0 && abs(diffToPlayerY) == 1) return;
		if (abs(diffToPlayerX) == 1 && diffToPlayerY == 0) return;

		if (rand() % 2 == 0 && diffToPlayerX != 0) {
			if (diffToPlayerX > 0)		prospectiveGridCoordX = world.enemy.gridCoords.x - 1;
			else if (diffToPlayerX < 0) prospectiveGridCoordX = world.enemy.gridCoords.x + 1;
		}
		else {
			if (diffToPlayerY == 0) {
				if (diffToPlayerX > 0)		prospectiveGridCoordX = world.enemy.gridCoords.x - 1;
				else if (diffToPlayerX < 0) prospectiveGridCoordX = world.enemy.gridCoords.x + 1;
			}

			if (diffToPlayerY > 0)		prospectiveGridCoordY = world.enemy.gridCoords.y - 1;
			else if (diffToPlayerY < 0) prospectiveGridCoordY = world.enemy.gridCoords.y + 1;
		}
	}
	// TODO: Finish this. The Other can go off map when avoidant.
	else if (world.enemy.actionState == ACTION_STATE_AVOIDANT) {
		if (diffToPlayerX > diffToPlayerY) {
			if		(diffToPlayerX > 0)	prospectiveGridCoordX = world.enemy.gridCoords.x + 1;
			else if (diffToPlayerX < 0) prospectiveGridCoordX = world.enemy.gridCoords.x - 1;

			
		}	else {
			if (diffToPlayerY > 0)		prospectiveGridCoordY = world.enemy.gridCoords.y + 1;
			else if (diffToPlayerY < 0) prospectiveGridCoordY = world.enemy.gridCoords.y - 1;
		}
	}

	if (isMapSpaceEmpty(world.player.worldCoordX, world.player.worldCoordY, prospectiveGridCoordX, prospectiveGridCoordY)) {
		world.enemy.gridCoords.x = prospectiveGridCoordX;
		world.enemy.gridCoords.y = prospectiveGridCoordY;
	}
}

#include "controls.h"

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (console.isOut) {
		console.addInput(codepoint);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
		console.flipOut();
		return;
	}

	if (console.isOut) {
		if (key == GLFW_KEY_BACKSPACE) {
			console.removeCharacter();
		}

		if (key == GLFW_KEY_ENTER) {
			console.submit();
		}

		return;
	}
	
	control ctlFunc = getControlFunc();
	if(ctlFunc) ctlFunc(action, key);
}

std::vector<std::string> splitString(std::string str, char delimiter) {
	std::vector<std::string> returnStrings;

	int count = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == delimiter) {
			returnStrings.push_back(std::string(str, i - count, count));
			count = 0;
		}
		else {
			count++;
		}
	}

	returnStrings.push_back(std::string(str, str.size() - count, count));

	return returnStrings;
}

void processConsoleCommand(std::string command) {
	std::vector<std::string> commandVector = splitString(command, ' ');
	
	if (commandVector[0] == "play") {
		world.camera.initializeForGrid();

		mode = MODE_PLAY;
		addTextToBox("Mode: Play", &eventTextBox);
	}

	if (commandVector[0] == "freecam") {
		mode = MODE_FREE_CAMERA;

		world.camera.initializeForGrid();
		addTextToBox("Mode: Free Camera", &eventTextBox);
	}

	if (commandVector[0] == "edit") {
		world.camera.initializeOverhead();

		mode = MODE_LEVEL_EDIT;
		addTextToBox("Mode: Level Edit", &eventTextBox);
	}

	if (commandVector[0] == "grid") {
		guidingGrid = !guidingGrid;
		addTextToBox("Guiding Grid: " + std::to_string(guidingGrid), &console.historyTextbox);
	}

	if (commandVector[0] == "orbit") {
		lightOrbit = !lightOrbit;
		addTextToBox("Light Orbit: " + std::to_string(lightOrbit), &eventTextBox);
	}

	if (commandVector[0] == "mat") {
		if (commandVector.size() < 3) return;

		setMaterial(commandVector[1], commandVector[2]);
	}
}

void processKeyboardInput(GLFWwindow *window) {
	// NOTE: Using the callback for free camera movement is super choppy,
	//		 Cause it's the only thing that involves holding down keys?
	if (mode == MODE_FREE_CAMERA) {
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

//void createLightCube() {
//	Mesh lightMesh;
//	
//	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
//		lightMesh.vertices.push_back(cubeVertices[i]);
//	}
//
//	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
//		lightMesh.indices.push_back(cubeIndices[i]);		
//	}
//
//	lightMesh.setupVAO();
//	lightMesh.shaderProgramID = lightShaderProgramID;
//	lightMesh.material = &materials.light;
//
//	models.lightCube.name = std::string("light");
//	models.lightCube.meshes.push_back(lightMesh);
//
//	//world.lightEntity.model = &models.lightCube;
//
//	//world.lightEntity.worldOffset = glm::vec3(-2.0f, -5.0f, 4.0f);
//}

void createGridFloorAndWallModels() {
	Mesh floorMesh;	
	Mesh wallMesh;

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		floorMesh.vertices.push_back(cubeVertices[i]);
		wallMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
		floorMesh.indices.push_back(cubeIndices[i]);
		wallMesh.indices.push_back(cubeIndices[i]);
	}
	
	floorMesh.setupVAO();
	floorMesh.shaderProgramID = regularShaderProgramID;
	floorMesh.material = &materials.silver;

	wallMesh.setupVAO();
	wallMesh.shaderProgramID = regularShaderProgramID;
	wallMesh.material = &materials.chrome;

	models.floorModel.name = std::string("floor");
	models.floorModel.meshes.push_back(floorMesh);

	models.wallModel.name = std::string("wall");
	models.wallModel.meshes.push_back(wallMesh);

	models.wallModel.scale(glm::vec3(0, 0, 1), 2.0f);
}

void createPlayerAndEnemyModels() {
	Mesh playerMesh;
	Mesh enemyMesh;

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		playerMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
		playerMesh.indices.push_back(cubeIndices[i]);
	}

	playerMesh.setupVAO();
	playerMesh.shaderProgramID = regularShaderProgramID;
	playerMesh.material = &materials.gold;

	models.player.name = std::string("player");
	models.player.meshes.push_back(playerMesh);
	models.player.scale(glm::vec3(0, 0, 1), 0.5f);

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		enemyMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(unsigned int); i++) {
		enemyMesh.indices.push_back(cubeIndices[i]);
	}

	enemyMesh.setupVAO();
	enemyMesh.shaderProgramID = regularShaderProgramID;
	enemyMesh.material = &materials.chrome;

	models.enemy.name = std::string("enemy");
	models.enemy.meshes.push_back(enemyMesh);
	models.enemy.scale(glm::vec3(0, 0, 1), 0.5f);
}

void drawGrid() {

	float zOffset = 0.0f;
	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		float yOffset = -0.5f * row;

		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			float xOffset = 0.5f * column;
			glm::vec3 worldOffset = glm::vec3(xOffset, yOffset, zOffset);

			if (world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[row][column] != 0) {
				models.wallModel.draw(worldOffset, UP, world.light);
			} else {
				models.floorModel.draw(worldOffset, UP, world.light);
			}
		}
	}
}

//glm::vec3 getEntityOffset(Entity entity) {
//	float zOffset = 0.5f;
//
//	float yOffset = -0.5f * entity.gridCoords.y;
//	float xOffset = 0.5f * entity.gridCoords.x;
//
//	return glm::vec3(xOffset, yOffset, zOffset);
//}
//
//glm::vec3 getPlayerModelCoords() {
//	glm::vec3 coords = getEntityOffset(world.player);
//	
//	if (mode == MODE_PLAY_FIRST_PERSON) coords.z += 0.25f;
//	
//	return coords;
//}

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

	//world.lightEntity.worldOffset.x = newX;
	//world.lightEntity.worldOffset.y = newY;

	world.light.currentDegrees = newDegrees;
}

// GRID LINES
unsigned int gridVAO_ID, gridVBO_ID;
unsigned int numGridVertices = 0;
void guidingGridSetup() {
	// TODO: create grid mesh

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

void drawGuidingGrid() {
	glUseProgram(regularShaderProgramID);

	glBindVertexArray(gridVAO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO_ID);

	glm::mat4 currentModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)); // TODO: this can just be the 0 mat4, right?
	setUniformMat4(regularShaderProgramID, "model", currentModel);

	setUniform3f(regularShaderProgramID, "objectDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	setUniform3f(regularShaderProgramID, "objectAmbient", glm::vec3(1.0f, 1.0f, 1.0f));

	glDrawArrays(GL_LINES, 0, numGridVertices);
}

void initMaterials() {
	// pulled from http://devernay.free.fr/cours/opengl/materials.html
	materials.emerald = Material("emerald", glm::vec3(0.0215f, 0.1745f, 0.0215f),
		glm::vec3(0.07568f, 0.61424f, 0.07568f),
		glm::vec3(0.633f, 0.727811f, 0.633f),
		0.6f);

	materials.chrome = Material("chrome", glm::vec3(0.25f, 0.25f, 0.25f),
		glm::vec3(0.4f, 0.4f, 0.4f),
		glm::vec3(0.774597f, 0.774597f, 0.774597f),
		0.6f);

	materials.silver = Material("silver", glm::vec3(0.19225, 0.19225, 0.19225),
		glm::vec3(0.50754, 0.50754, 0.50754),
		glm::vec3(0.508273, 0.508273, 0.508273),
		0.4f);

	materials.gold = Material("gold", glm::vec3(0.24725, 0.1995, 0.0745),
		glm::vec3(0.75164, 0.60648, 0.22648),
		glm::vec3(0.628281, 0.555802, 0.366065),
		0.4f);

	materials.blackRubber = Material("blackRubber", glm::vec3(0.02, 0.02, 0.02),
		glm::vec3(0.01, 0.01, 0.01),
		glm::vec3(0.4, 0.4, 0.4),
		.078125f);

	materials.ruby = Material("ruby", glm::vec3(0.1745, 0.01175, 0.01175),
		glm::vec3(0.61424, 0.04136, 0.04136),
		glm::vec3(0.727811, 0.626959, 0.626959),
		0.6f);

	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
	materials.light.name = "whiteLight";
	materials.light.diffuse = white;
	materials.light.ambient = white;
	materials.light.specular = white;
	materials.light.shininess = 1.0f;
}

void drawPlayer() {

	//world.player.worldOffset = getEntityOffset(world.player);
	world.player.draw(world.light);
}

void drawEnemy() {

	if (world.enemy.worldCoordX != world.player.worldCoordX ||
		world.enemy.worldCoordY != world.player.worldCoordY) return; 
	
	//world.enemy.worldOffset = getEntityOffset(world.enemy);
	world.enemy.draw(world.light);
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
	textShaderProgramID		= createShaderProgram("textVertexShader.vert", "textFragmentShader.frag");

	// ------------- FONTS   -------------
	Font arial = Font("arial.ttf", textShaderProgramID);

	resetProjectionMatrices();

	// initializing viewport and setting callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCharCallback(window, character_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);

	world.light.pos = glm::vec3(-2.0f, -5.0f, 4.0f);
	
	createGridFloorAndWallModels();
	createPlayerAndEnemyModels();

	// set seed and generate map
	unsigned int seed = (unsigned int)time(NULL); // seconds since Jan 1, 2000
	srand(seed);
	addTextToBox("seed: " + std::to_string(seed), &eventTextBox);
	
	generateWorldMap(&world);

	// CHARACTER INITIALIZATION
	world.player.worldCoordX = PLAYER_WORLD_START_X;
	world.player.worldCoordY = PLAYER_WORLD_START_Y;
	world.player.gridCoords.x = PLAYER_GRID_START_X;
	world.player.gridCoords.y = PLAYER_GRID_START_Y;
	world.player.gridCoords.z = 1;
	world.player.strength = 1;
	world.player.hitPoints = 20;
	world.player.directionFacing = UP;

	world.enemy.worldCoordX = PLAYER_WORLD_START_X;
	world.enemy.worldCoordY = PLAYER_WORLD_START_Y;
	world.enemy.gridCoords.x = 1;
	world.enemy.gridCoords.y = 2;
	world.enemy.gridCoords.z = 1;
	world.enemy.hitPoints = 3;
	world.enemy.strength = 1;

	lastFrameTime = (float)glfwGetTime();

	// need alpha blending for text transparency
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	world.camera.initializeForGrid();

	guidingGridSetup();

	console.setup();
	initMaterials();

	world.player.model = &models.player;
	world.enemy.model = &models.enemy;

	// game loop
	while (!glfwWindowShouldClose(window)) {
		processKeyboardInput(window);
		
		glfwPollEvents();

		//if (mode == MODE_PLAY_FIRST_PERSON) world.camera.position = getPlayerModelCoords();
		
		refreshView();

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);

		glDepthFunc(GL_LESS);
		if(guidingGrid)	drawGuidingGrid();
		if(lightOrbit) moveLightAroundOrbit(deltaTime);
		
		//world.lightEntity.draw(world.light);
		drawGrid();
		if (mode != MODE_PLAY_FIRST_PERSON) drawPlayer();
		drawEnemy();

		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		console.draw(deltaTime, &arial);
		drawTextBox(&eventTextBox, &arial);

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}