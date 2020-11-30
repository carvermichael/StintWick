#include <windows.h>
// suppress warnings from non-standard lib external sources
#pragma warning (push, 0)
#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning (pop)

#include <time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <map>

#include "math.h"
#include "utils.h"
#include "constants.h"

#include "global_manip.h"

#include "model.h"

Models models;
Materials materials;

#include "camera.h"
#include "worldState.h"
#include "shader.h"
#include "textBox.h"
#include "console.h"
#include "editorUI.h"
#include "levels.h"

// TODO: Should all of this go on the heap???
	// Random Global State -- some subset of this should go in the worldState
	
	unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;
	unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
	unsigned int editor_mode = EDITOR_MODE_ENEMY;
	bool blinkMeshes = true;

	float globalDeltaTime = 0.0f;
	float lastFrameTime = 0.0f;
	float lastCursorX = 400;
	float lastCursorY = 300;
	bool firstMouse = true;
	
	//bool pause = true;
	bool lightOrbit = false;
	bool guidingGrid = false;
	my_ivec3 pauseCoords[30];
	bool outlineOnly = false;
	unsigned int levelCount;
	unsigned int currentLevel = 0;
	unsigned int currentEnemyTypeSelection = 0;
	
	// OpenGL Stuff
	unsigned int regularShaderProgramID;
	unsigned int lightShaderProgramID;
	unsigned int UIShaderProgramID;
	unsigned int textShaderProgramID;
	glm::mat4 projection;

	// Larger Game Pieces

	EnemyStrats enemyStrats;

	EditorUI editorUI;
	Console console;
	WorldState *currentWorldState;
	WorldState *worldStates[2];
	Camera camera;

	Level levels[MAX_LEVELS];
	UI_Rect pauseThingy;

	#include "shapeData.h"

	Textbox eventTextBox = {};
	Textbox fpsBox = {};

#include "controls.h"


// UNIFORM SETTING
void refreshProjection() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 300.0f);

	//printGLMMat4(projection);

	setUniformMat4(regularShaderProgramID, "projection", projection);
	setUniformMat4(lightShaderProgramID, "projection", projection);

	glm::mat4 UIProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	
	setUniformMat4(textShaderProgramID, "projection", UIProjection);
	setUniformMat4(UIShaderProgramID, "projection", UIProjection);
}

void refreshView() {
	
	my_mat4 view = camera.generateMyView();

	setUniformMat4(regularShaderProgramID, "view", view);
	setUniformMat4(lightShaderProgramID, "view", view);

	setUniform3f(regularShaderProgramID, "viewPos", camera.position);
	setUniform3f(lightShaderProgramID, "viewPos", camera.position);
}

void refreshLights() {
	for (int i = 0; i < MAX_LIGHTS; i++) {
		std::string lightsCurrent = "lights[" + std::to_string(i) + "].current";
		setUniformBool(regularShaderProgramID, lightsCurrent.c_str(), currentWorldState->lights[i].current);
		
		if (!currentWorldState->lights[i].current) continue;

		std::string lightsPos		= "lights[" + std::to_string(i) + "].pos";
		std::string lightsAmbient	= "lights[" + std::to_string(i) + "].ambient";
		std::string lightsDiffuse	= "lights[" + std::to_string(i) + "].diffuse";
		std::string lightsSpecular	= "lights[" + std::to_string(i) + "].specular";
		
		setUniform3f(regularShaderProgramID,	lightsPos.c_str(),			currentWorldState->lights[i].pos);
		setUniform3f(regularShaderProgramID,	lightsAmbient.c_str(),		currentWorldState->lights[i].ambient);
		setUniform3f(regularShaderProgramID,	lightsDiffuse.c_str(),		currentWorldState->lights[i].diffuse);
		setUniform3f(regularShaderProgramID,	lightsSpecular.c_str(),		currentWorldState->lights[i].specular);
	}	
}

void setUniformBool(unsigned int shaderProgramID, const char *uniformName, bool value) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform1i(location, value);
}

void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform1f(location, value);
}

void setUniform3f(unsigned int shaderProgramID, const char *uniformName, my_vec3 my_vec3) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform3f(location, my_vec3.x, my_vec3.y, my_vec3.z);
}

void setUniform4f(unsigned int shaderProgramID, const char *uniformName, my_vec4 my_vec4) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform4f(location, my_vec4.x, my_vec4.y, my_vec4.z, my_vec4.w);
}

void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	my_mat4 myMat4 = my_mat4(mat4);
	glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat *)&myMat4);
}

void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, my_mat4 mat4) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat *)&mat4);
}


// RANDOM UTILS
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

// GLOBAL STATE STUFF
// TODO: bug fix -- bullets move fast enough such that they cross multiple integer boundaries, resulting in frequent teleporting through walls and enemies (at 60 fps)
//			     -- in wall collision check, need to check every integer boundary crossed, moving initial position to final position
my_vec2 adjustForWallCollisions(AABB entityBounds, my_vec2 move, bool *collided) {
	float moveX = move.x;
	float moveY = move.y;

	*collided = false;

	float finalOffsetX = entityBounds.AX;
	float finalOffsetY = entityBounds.AY;

	int playerAXFloor = (int)entityBounds.AX;
	int playerBXFloor = (int)entityBounds.BX;
	int playerAYFloor = (int)entityBounds.AY;
	int playerBYFloor = (int)entityBounds.BY;

	if (moveX < 0) {
		if (finalOffsetX + moveX < playerAXFloor &&							// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(currentWorldState->grid[playerAXFloor - 1][-playerAYFloor] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
				currentWorldState->grid[playerAXFloor - 1][-playerBYFloor] == WALL)) {
			finalOffsetX = (float)playerAXFloor;
			*collided = true;
		}
		else {
			finalOffsetX += moveX;
		}
	}
	if (moveX > 0) {
		if ((int)(entityBounds.BX + moveX) > playerBXFloor &&				// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(currentWorldState->grid[playerBXFloor + 1][-playerAYFloor] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
				currentWorldState->grid[playerBXFloor + 1][-playerBYFloor] == WALL)) {
			finalOffsetX = (float)(playerBXFloor - 0.01f);					// hacky solution: flooring the BX bounds doesn't work when BX is exactly on the integer line
			*collided = true;
		}
		else {
			finalOffsetX += moveX;
		}
	}

	if (moveY < 0) {
		if ((int)(entityBounds.BY + moveY) <= playerBYFloor - 1 &&			// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(currentWorldState->grid[playerBXFloor][-playerBYFloor + 1] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
				currentWorldState->grid[playerAXFloor][-playerBYFloor + 1] == WALL)) {
			finalOffsetY = (float)playerBYFloor + 0.001f;
			*collided = true;
		}
		else {
			finalOffsetY += moveY;
		}
	}

	if (moveY > 0) {
		if ((int)(entityBounds.BY + moveY) > playerBYFloor &&				// checks to see if player will hit an integer boundary (walls only occur on integer boundries)
			(currentWorldState->grid[playerBXFloor][-playerAYFloor - 1] == WALL ||		// effectively checks to see if there's a wall where top left will hit or where bottom left will hit
				currentWorldState->grid[playerAXFloor][-playerAYFloor - 1] == WALL)) {
			finalOffsetY = (float)playerAYFloor - 0.001f;					// hacky solution: flooring the BX bounds doesn't work when BX is exactly on the integer line
			*collided = true;
		}
		else {
			finalOffsetY += moveY;
		}
	}

	return my_vec2(finalOffsetX, finalOffsetY);
}

void createParticleEmitter(my_vec3 newPos) {

	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (!currentWorldState->particleEmitters[i].current) {
			currentWorldState->particleEmitters[i].init(newPos, &models.bulletPart, currentWorldState->lights);
			return;
		}
	}

	printf("ParticleEmitter array full! Ah!\n");
}

void setPauseCoords() {
	for (int i = 0; i < 30; i++) {
		pauseCoords[i].x = rand() % currentScreenWidth;
		pauseCoords[i].y = rand() % currentScreenHeight;
		pauseCoords[i].z = rand() % 200;
	}
}

void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed) {
	bool foundBullet = false;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!currentWorldState->enemyBullets[i].current) {
			currentWorldState->enemyBullets[i].init(worldOffset,
				my_vec2(dirVec.x, dirVec.y),
				&models.enemyBullet, speed);
			foundBullet = true;
			break;
		}
	}

	if (!foundBullet) printf("Bullet array full! Ah!\n");
}

unsigned int getCurrentLevel() {
	return currentLevel;
}

unsigned int getEnemySelection() {
	return currentEnemyTypeSelection;
}

void goBackOneLevel() {
	if (currentLevel == 0) currentLevel = levelCount - 1;
	else currentLevel--;

	loadCurrentLevel();
}

void goForwardOneLevel() {
	currentLevel++;
	if (currentLevel >= levelCount) currentLevel = 0;

	loadCurrentLevel();
}

// This is starting to become a world state reset...
void loadCurrentLevel(WorldState *worldState) {
	// This memory wipe doesn't work now that recorded inputs are in the worldState struct.
	// Maybe I should just keep a pointer to inputs in the worldstate, then pass that pointer
	// with the level when calling resetToLevel.
	// SecureZeroMemory(currentWorldState, sizeof(WorldState));
	
	eventTextBox.clearTextBox();

	// reset camera
	if (worldState->mode == MODE_LEVEL_EDIT) {
		camera.initOverhead(worldState->gridSizeX, worldState->gridSizeY);
	}
	else {
		camera.initOnPlayer(worldState->player.worldOffset);
	}

 	worldState->resetToLevel(&levels[currentLevel], &eventTextBox);
}

void loadCurrentLevel() {
	// This memory wipe doesn't work now that recorded inputs are in the worldState struct.
	// Maybe I should just keep a pointer to inputs in the worldstate, then pass that pointer
	// with the level when calling resetToLevel.
	// SecureZeroMemory(currentWorldState, sizeof(WorldState));

	eventTextBox.clearTextBox();

	// reset camera
	if (currentWorldState->mode == MODE_LEVEL_EDIT) {
		camera.initOverhead(currentWorldState->gridSizeX, currentWorldState->gridSizeY);
	}
	else {
		camera.initOnPlayer(currentWorldState->player.worldOffset);
	}

	currentWorldState->resetToLevel(&levels[currentLevel], &eventTextBox);
}

void deleteCurrentLevel() {
	for (unsigned int i = currentLevel; i < levelCount; i++) {
		levels[i] = levels[i + 1];
		levels[i + 1].initialized = false;
	}
	if (currentLevel == levelCount - 1) {
		currentLevel = 0;
	}
	else {
		currentLevel--;
	}

	levelCount--;
	loadCurrentLevel();
}

void goBackOneEnemyType() {
	int numEnemyTypes = sizeof(enemyStrats) / sizeof(enemyStrats.follow);
	
	if (currentEnemyTypeSelection == 0) currentEnemyTypeSelection = numEnemyTypes - 1;
	else currentEnemyTypeSelection--;
}

void goForwardOneEnemyType() {
	unsigned int numEnemyTypes = sizeof(enemyStrats) / sizeof(enemyStrats.follow);

	currentEnemyTypeSelection++;
	if (currentEnemyTypeSelection >= numEnemyTypes) currentEnemyTypeSelection = 0;
}

void addEnemyToLevel(int type, my_ivec2 gridCoords) {
	levels[currentLevel].addEnemy(type, gridCoords, &eventTextBox);
}

void addEnemyToWorld(int type, my_ivec2 gridCoords) {
	if (currentWorldState->numEnemies >= MAX_ENEMIES) {
		printf("ERROR: Max enemies reached.\n");
		eventTextBox.addTextToBox("ERROR: Max enemies reached.");
		return;
	}

	EnemyStrat	*strat	= &enemyStrats.follow;
	if (type == 0) {
		strat = &enemyStrats.shoot;
	}
	else if (type == 1) {		
		strat = &enemyStrats.follow;
	}
	
	Material	*mat	= &materials.mats[type];

	currentWorldState->enemies[currentWorldState->numEnemies].init(gridCoordsToWorldOffset(my_ivec3(gridCoords.x, gridCoords.y, 1)), &models.enemy, mat, strat);
	currentWorldState->numEnemies++;
}

// TODO: need addWallToLevel as well
void addWallToWorld(my_ivec2 gridCoords) {
	//// v2
	//world->wallLocations[world->numWalls].x = gridCoords.x;
	//world->wallLocations[world->numWalls].y = gridCoords.y;

	//world->numWalls++;

	// v3
	currentWorldState->grid[gridCoords.x][gridCoords.y] = WALL;
}

void toggleEditorMode() {
	if (editor_mode == EDITOR_MODE_ENEMY) editor_mode = EDITOR_MODE_WALL;
	else if (editor_mode == EDITOR_MODE_WALL) editor_mode = EDITOR_MODE_ENEMY;
}

int getEditorMode() {
	return editor_mode;
}

my_ivec3 cameraCenterToGridCoords() {
	my_vec3 startingPos = camera.position;
	my_vec3 dirVec = camera.front;

	my_vec3 currentPos = startingPos;

	float rayStep = 0.1f;

	while (currentPos.z > 0) {
		currentPos.x += dirVec.x * rayStep;
		currentPos.y += dirVec.y * rayStep;
		currentPos.z += dirVec.z * rayStep;
	}

	my_ivec3 gridCoords = worldOffsetToGridCoords(currentPos);

	return gridCoords;
}

void processConsoleCommand(std::string command) {
	std::vector<std::string> commandVector = splitString(command, ' ');

	if (commandVector[0] == "play") {
		//camera.initOnPlayer(currentWorldState->player.worldOffset);

		currentWorldState->mode = MODE_PLAY;
		worldStates[0]->mode = MODE_PLAY;
		worldStates[1]->mode = MODE_PLAY;

		eventTextBox.addTextToBox("Mode: Play");
	}

	if (commandVector[0] == "freecam") {
		currentWorldState->mode = MODE_FREE_CAMERA;

		camera.initOnPlayer(currentWorldState->player.worldOffset);
		eventTextBox.addTextToBox("Mode: Free Camera");
	}

	if (commandVector[0] == "overhead") {
		camera.initOverhead(currentWorldState->gridSizeX, currentWorldState->gridSizeY);
	}

	if (commandVector[0] == "edit") {
		camera.initOverhead(currentWorldState->gridSizeX, currentWorldState->gridSizeY);

		currentWorldState->mode = MODE_LEVEL_EDIT;
		eventTextBox.addTextToBox("Mode: Level Edit");
	}

	if (commandVector[0] == "grid") {
		guidingGrid = !guidingGrid;
		console.historyTextbox.addTextToBox("Guiding Grid: " + std::to_string(guidingGrid));
	}

	if (commandVector[0] == "orbit") {
		lightOrbit = !lightOrbit;
		eventTextBox.addTextToBox("Light Orbit: " + std::to_string(lightOrbit));
	}

	if (commandVector[0] == "level count") {
		eventTextBox.addTextToBox("levelCount: " + std::to_string(levelCount));
	}

	if (commandVector[0] == "newLevel") {

		if (commandVector.size() < 3) {
			eventTextBox.addTextToBox("error: missing args gridSizeX and gridSizeY");
			return;
		}

		unsigned int gridSizeX = std::stoi(commandVector[1], nullptr, 10);
		unsigned int gridSizeY = std::stoi(commandVector[2], nullptr, 10);

		levelCount = addLevel(levels, levelCount, gridSizeX, gridSizeY);
		currentLevel = levelCount - 1;
		loadCurrentLevel();
	}

	if (commandVector[0] == "mat") {
		if (commandVector.size() < 3) {
			eventTextBox.addTextToBox("error: missing args for material and model");
			return;
		}

		setMaterial(commandVector[1], commandVector[2], &materials, &models, &console);
	}
}

// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	currentScreenHeight = height;

	currentScreenWidth = (INITIAL_SCREEN_WIDTH * currentScreenHeight) / INITIAL_SCREEN_HEIGHT;

	glViewport((width - currentScreenWidth) / 2, 0, currentScreenWidth, currentScreenHeight);

	console.refresh((float)currentScreenWidth, (float)currentScreenHeight);
	editorUI.refresh((float)currentScreenWidth, (float)currentScreenHeight);
	pauseThingy.setBounds(AABB(0.0f, (float)currentScreenWidth, (float)currentScreenHeight, 0.0f));
	setPauseCoords();

	refreshProjection();
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (console.isOut) {
		console.addInput((char)codepoint);
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
	if (ctlFunc) ctlFunc(action, key, globalDeltaTime);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (currentWorldState->mode != MODE_LEVEL_EDIT) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// check in window
		if (lastCursorX < 0 || lastCursorX > currentScreenWidth ||
			lastCursorY < 0 || lastCursorY > currentScreenHeight) {
			eventTextBox.addTextToBox("Cursor is off screen, &eventTextBox");
			return;
		}

		// Origin for click coordinates are top-left, y increases downward.
		// Thus, need to get the screen height complement to accurately judge clicks
		// w.r.t. UI elements (which have their origin in bottom left, and y increases upward).

		// check in editor box
		if (currentWorldState->mode == MODE_LEVEL_EDIT) {
			if (editorUI.click(my_vec2(lastCursorX, glm::abs(lastCursorY - currentScreenHeight)))) return;
		}

		// TODO: Raycast from click coords instead of camera center.
		// Slight hack to get a working way to place enemies before groking a raycast from click coords. (carver - 9-09-2020)
		my_ivec3 gridCoords = cameraCenterToGridCoords();

		if (editor_mode == EDITOR_MODE_ENEMY) {
			// place enemy
		
			// TODO: maybe find a better way to keep world and level in sync
			//			- may end up with a staging level struct at some point
			addEnemyToWorld(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
			addEnemyToLevel(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
		}
		else if (editor_mode == EDITOR_MODE_WALL) {
			addWallToWorld(my_ivec2(gridCoords.x, gridCoords.y));
		}		
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		// get grid coords
		my_ivec3 gridCoords = cameraCenterToGridCoords();
		my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);

		// remove target enemy from world
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (currentWorldState->enemies[i].current) {
				if (worldOffset.x == currentWorldState->enemies[i].worldOffset.x &&
					worldOffset.y == currentWorldState->enemies[i].worldOffset.y) {
					currentWorldState->enemies[i].current = false;
				}
			}
		}

		// remove target enemy from level
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (levels[currentLevel].enemies[i].gridX == gridCoords.x &&
				levels[currentLevel].enemies[i].gridY == gridCoords.y) {
				levels[currentLevel].removeEnemy(i);
			}
		}
	}
}

void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	// TODO: Figure out how to lockdown the cursor such that the coordinate args can't go outside the viewport
	//		 Clamping won't work.
	
	if (currentWorldState->mode == MODE_FREE_CAMERA) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (firstMouse)
		{
			lastCursorX = (float)xPos;
			lastCursorY = (float)yPos;
			firstMouse = false;
		}

		float xOffset = (float)(xPos - lastCursorX);
		float yOffset = (float)(lastCursorY - yPos);

		camera.adjustYawAndPitch(xOffset, yOffset);		
	}
	else if (currentWorldState->mode == MODE_LEVEL_EDIT) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstMouse = true;

		// TODO: light up moused over grid piece

	} else if (currentWorldState->mode == MODE_PLAY) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstMouse = true;
	}

	lastCursorX = (float)xPos;
	lastCursorY = (float)yPos;
}

// MODEL CREATION
void createBulletModel() {
	Mesh bulletMesh = Mesh(&regularShaderProgramID, &materials.grey);
	models.bullet.name = std::string("bullet");
	models.bullet.meshes.push_back(bulletMesh);
	models.bullet.scale(my_vec3(0.5f));

	Mesh enemyBulletMesh = Mesh(&regularShaderProgramID, &materials.gold);
	models.enemyBullet.name = std::string("enemyBullet");
	models.enemyBullet.meshes.push_back(enemyBulletMesh);
	models.enemyBullet.scale(my_vec3(0.5f));

	Mesh bulletPartMesh = Mesh(&regularShaderProgramID, &materials.grey);
	models.bulletPart.name = std::string("bulletPart");
	models.bulletPart.meshes.push_back(bulletPartMesh);
	models.bulletPart.scale(my_vec3(0.15f));
}

void createGridFloorAndWallModels() {
	Mesh floorMesh = Mesh(&regularShaderProgramID, &materials.blackRubber);
	models.floorModel.name = std::string("floor");
	models.floorModel.meshes.push_back(floorMesh);
	
	Mesh wallMesh = Mesh(&regularShaderProgramID, &materials.yellow);
	models.wall.name = std::string("wallLeft");
	models.wall.meshes.push_back(wallMesh);
}

void createPlayerAndEnemyModels() {
	Mesh playerMesh = Mesh(&regularShaderProgramID, &materials.emerald);
	models.player.name = std::string("player");
	models.player.meshes.push_back(playerMesh);
	
	Mesh enemyMesh = Mesh(&regularShaderProgramID, &materials.ruby);
	models.enemy.name = std::string("enemy");
	models.enemy.meshes.push_back(enemyMesh);
	models.enemy.scale(my_vec3(1.0f, 1.0f, 0.5f));
}

// DRAWS, REGULAR
void drawProspectiveOutline() {
	// turn current center of screen into grid coordinates
	my_ivec3 gridCoords = cameraCenterToGridCoords();

	my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);
	worldOffset.z = 1.05f; // extra for outline visibility

	// draw just an outline in that space
	models.enemy.drawOnlyOutline(worldOffset);
}

// GRID LINES
unsigned int gridVAO_ID, gridVBO_ID;
unsigned int numGridVertices = 0;
void guidingGridSetup() {
	// TODO: create grid mesh
	//		 note: can't do this is with current mesh setup, as those meshes are made for triangle drawing (these are just lines)

	float lowerZ = 0.05f;
	float upperZ = 4.05f;
	float zStep	 = 2.0f;

	float lowerY =  -30.05f;
	float upperY =  0.05f;
	float yStep  =  1.0f;

	float lowerX =  0.05f;
	float upperX =	30.05f;
	float xStep	 =  1.0f;

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

	glm::mat4 currentModel = glm::mat4(1.0f);
	setUniformMat4(regularShaderProgramID, "model", currentModel);

	setUniform3f(regularShaderProgramID, "objectDiffuse", my_vec3(1.0f, 1.0f, 1.0f));
	setUniform3f(regularShaderProgramID, "objectAmbient", my_vec3(1.0f, 1.0f, 1.0f));

	glDrawArrays(GL_LINES, 0, numGridVertices);
}

void moveLightAroundOrbit(float deltaTime) {
	float radius = 35.0f;
	float speed = 90.0f; // degrees / second
	float degreesMoved = speed * deltaTime;

	float midGridX = (float)currentWorldState->gridSizeX / 2;
	float midGridY = -(float)currentWorldState->gridSizeY / 2;

	float newDegrees = currentWorldState->lights[0].currentDegrees + degreesMoved;
	if (newDegrees > 360) newDegrees -= 360;

	float newX = glm::cos(glm::radians(newDegrees)) * radius + midGridX;
	float newY = glm::sin(glm::radians(newDegrees)) * radius + midGridY;

	currentWorldState->lights[0].pos.x = newX;
	currentWorldState->lights[0].pos.y = newY;

	//world->lightEntity.worldOffset.x = newX;
	//world->lightEntity.worldOffset.y = newY;

	currentWorldState->lights[0].currentDegrees = newDegrees;
}

DWORD WINAPI imageLoadAndFree(LPVOID lpParam) {
	// Basic usage (see HDR discussion below for HDR usage):
	//    int x,y,n;
	//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
	//    // ... process data if not NULL ...
	//    // ... x = width, y = height, n = # 8-bit components per pixel ...
	//    // ... replace '0' with '1'..'4' to force that many components per pixel
	//    // ... but 'n' will always be the number that it would have been if you said 0
	//    stbi_image_free(data)
	//
	// Standard parameters:
	//    int *x                 -- outputs image width in pixels
	//    int *y                 -- outputs image height in pixels
	//    int *channels_in_file  -- outputs # of image components in image file
	//    int desired_channels   -- if non-zero, # of image components requested in result
	int count = 0;

	while (true) {
		printf("loading %d \n", count);

		int x, y, n;
		unsigned char *data = stbi_load("botw.jpg", &x, &y, &n, 0);
		printf("...loaded...");

		stbi_image_free(data);
		printf("freed!\n");
		count++;
	}
}

void liveReloadFragmentShader(WIN32_FILE_ATTRIBUTE_DATA *prevFragmentShaderFileData, WIN32_FILE_ATTRIBUTE_DATA *fragmentShaderFileData) {
	GetFileAttributesExA(
		"fragmentShader.frag",
		GetFileExInfoStandard,
		fragmentShaderFileData
	);

	if (fragmentShaderFileData->ftLastWriteTime.dwLowDateTime != prevFragmentShaderFileData->ftLastWriteTime.dwLowDateTime) {
		printf("File updated.\n");
		regularShaderProgramID = createShaderProgram("vertexShader.vert", "fragmentShader.frag");

		refreshProjection();
		refreshView();
		prevFragmentShaderFileData = fragmentShaderFileData;
	}
}

int main() {
	WorldState *worldState1 = (WorldState *)VirtualAlloc(0, sizeof(WorldState), MEM_COMMIT, PAGE_READWRITE);
	WorldState *worldState2 = (WorldState *)VirtualAlloc(0, sizeof(WorldState), MEM_COMMIT, PAGE_READWRITE);
	
	worldStates[0] = worldState1;
	worldStates[1] = worldState2;

	currentWorldState = worldState1;
	worldState2->globalOffset = my_vec3(75.0f, 0.0f, 0.0f);

	// ------------ INIT STUFF -------------

	// initialization of glfw and glad libraries, window creation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE__	

	glfwWindowHint(GLFW_SAMPLES, 4); // for MSAA, takes 4 samples per pixel, bufferSize *= 4
	GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "Stint Wick", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD");
		return -1;
	}

	// ------------- SHADERS -------------	
	regularShaderProgramID	= createShaderProgram("vertexShader.vert", "fragmentShader.frag");
	lightShaderProgramID	= createShaderProgram("lightVertexShader.vert", "lightFragmentShader.frag");
	UIShaderProgramID		= createShaderProgram("UIVertexShader.vert", "UIFragmentShader.frag");
	textShaderProgramID		= createShaderProgram("textVertexShader.vert", "textFragmentShader.frag");

	// ------------- FONTS   -------------
	Font arial = Font("arial.ttf", textShaderProgramID);

    refreshProjection();

	// initializing viewport and setting callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCharCallback(window, character_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT);

	// need alpha blending for text transparency
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// MODELS
	guidingGridSetup();
	createGridFloorAndWallModels();
	createPlayerAndEnemyModels();
    createBulletModel();

	console.setup(UIShaderProgramID, (float)currentScreenWidth, (float)currentScreenHeight, &arial);
	fpsBox.x = (float)(currentScreenWidth - 150);
	fpsBox.y = (float)(currentScreenHeight - 50);

	levelCount = loadLevelsV2(levels);
	currentLevel = 0;

	currentWorldState = worldState1;
	loadCurrentLevel(worldState1);

	currentWorldState = worldState2;
	loadCurrentLevel(worldState2);
	
	lastFrameTime = (float)glfwGetTime();

    float deltaTime = 0.0f;
	
    float targetFrameTime60 = 1.0f / 60.0f;
	float targetFrameTime90 = 1.0f / 90.0f;

	float targetFrameTime = targetFrameTime60;

	editorUI.setup(UIShaderProgramID, &arial, (float)currentScreenWidth, (float)currentScreenHeight);

	//world->player.blinking = true;

	pauseThingy.initialized = false;
	pauseThingy.setup(UIShaderProgramID);
	pauseThingy.color = my_vec4(0.1f, 0.1f, 0.1f, 0.7f);
	pauseThingy.setBounds(AABB(0.0f, (float)currentScreenWidth, (float)currentScreenHeight, 0.0f));
	setPauseCoords();

	outlineOnly = true;

	GLFWgamepadstate gamepadState;
	
	// game loop
	WIN32_FILE_ATTRIBUTE_DATA prevFragmentShaderFileData;
	GetFileAttributesExA(
		"fragmentShader.frag",
		GetFileExInfoStandard,
		&prevFragmentShaderFileData
	);

	WIN32_FILE_ATTRIBUTE_DATA fragmentShaderFileData = prevFragmentShaderFileData;

	// MULTI-THREAD EXPERIMENT
	//DWORD threadId;
	//HANDLE threadHandle = CreateThread(
	//	NULL,                   // default security attributes
	//	0,                      // use default stack size  
	//	imageLoadAndFree,       // thread function name
	//	0,			            // argument to thread function 
	//	0,                      // use default creation flags 
	//	&threadId);				// returns the thread identifier 

	while (!glfwWindowShouldClose(window)) {

		liveReloadFragmentShader(&prevFragmentShaderFileData, &fragmentShaderFileData);

		// -- INPUT --
		if(!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepadState)) {	/* TODO: logging */	};
		
		processKeyboardInput(window, deltaTime);
        glfwPollEvents();
		
		// -- UPDATE -- 
		currentWorldState = worldState1;
		currentWorldState->update(&gamepadState, deltaTime);

		currentWorldState = worldState2;
		currentWorldState->update(&gamepadState, deltaTime);
		//if (currentWorldState->mode != MODE_FREE_CAMERA) camera.update(deltaTime, currentWorldState->player.worldOffset);
		if (lightOrbit) moveLightAroundOrbit(deltaTime);
		console.update(deltaTime);

		// -- DRAW --
		
		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.65f, 0.65f, 0.65f, 1.0f);

		// World1
		currentWorldState = worldState1;
		refreshView();
		refreshLights();

		glDepthFunc(GL_LESS);
		if(guidingGrid)	drawGuidingGrid();
		setUniform3f(regularShaderProgramID, "playerPos", currentWorldState->player.worldOffset); // for fog stuff -- not sure where this should be
		currentWorldState->draw();
		if (currentWorldState->mode == MODE_LEVEL_EDIT) drawProspectiveOutline();

		// World2
		currentWorldState = worldState2;
		refreshLights();
		currentWorldState->draw();
		
		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		eventTextBox.drawTextBox(&arial);
		fpsBox.drawTextBox(&arial);
		if(currentWorldState->mode == MODE_LEVEL_EDIT) editorUI.draw();
		else if (currentWorldState->mode == MODE_PAUSED) {
			//pauseThingy.draw();
			
			//for (int i = 0; i < 30; i++) {
			//	drawText(&arial, "PRESS START", (float)pauseCoords[i].x, (float)pauseCoords[i].y, pauseCoords[i].z * 0.005f, my_vec3(1.0f));
			//}
		}
		console.draw();

		// -- FRAME TIMING --
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;	
		
		if (currentWorldState->mode != MODE_REPLAY) {
			if (deltaTime < targetFrameTime) {
				int timeToSleepMS = (int)(1000.0f * (targetFrameTime - deltaTime));

				if (timeToSleepMS > 1) {
					// TODO: make sure that windows is able to sleep at the 1ms level (HH code does this somewhere)

					Sleep(timeToSleepMS);
				}

				while (deltaTime < targetFrameTime) {
					deltaTime = (float)glfwGetTime() - lastFrameTime;
				}
			}
			else {
				printf("MISSED FRAME! AHH\n"); // TODO: logging
			}
		}

		float frameTime = deltaTime * 1000.0f;

		std::stringstream stream;
		stream << "FrameTime(ms): " << std::setprecision(4) << frameTime;
		drawText(&arial, stream.str(), 0, (float) currentScreenHeight - 30.0f, 0.5f, my_vec3(1.0f));
		
		globalDeltaTime = deltaTime;

		lastFrameTime = (float)glfwGetTime();

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}
