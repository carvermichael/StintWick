#include <windows.h>
// suppress warnings from non-standard lib external sources
#pragma warning (push, 0)
#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
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
#include "constants.h"

#include "randomUtils.h"

#include "global_manip.h"

#include "model.h"
#include "camera.h"
#include "worldState.h"
#include "shader.h"
#include "textBox.h"
#include "console.h"
#include "levels.h"
#include "editor.h"
	
unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;
unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
	
/*
	globalModes
	-- free cam
	-- level edit
	-- global play (used to make the camera follow the player during play)

	worldstate modes
	-- play
	-- pause
	-- replay

	editor modes
	-- placement type (enemy v wall)
	-- enemy type to be placed
*/

unsigned int globalMode = MODE_GLOBAL_PLAY;
		
float globalDeltaTime = 0.0f;
float lastFrameTime = 0.0f;
float lastCursorX = 400;
float lastCursorY = 300;
bool firstMouse = true;
	
bool lightOrbit = false;
	
unsigned int levelCount;
unsigned int currentLevelIndex = 0;
unsigned int enemyTypeSelection = 0;
unsigned int editorMode = EDITOR_MODE_ENEMY;

// OpenGL Stuff
unsigned int regularShaderProgramID;
unsigned int lightShaderProgramID;
unsigned int UIShaderProgramID;
unsigned int textShaderProgramID;
glm::mat4 projection;

// Larger Game Pieces
Models models;
Materials materials;
EnemyStrats enemyStrats;

Console console;
WorldState *world;
Editor editor;
Camera camera;
Font arial;

GLFWgamepadstate prevGamepadState;

InputRecord prevInputRecord;
InputRecord recordedInput[INPUT_MAX];
int currentInputIndex = 0;

Level levels[MAX_LEVELS];
	
Textbox eventTextBox = {};
Textbox fpsBox = {};

#include "controls.h"

// TODO: there's probably a better place for this... --> probably just in another header file, really
my_vec2 adjustForWallCollisions(AABB entityBounds, float moveX, float moveY, bool *collided) {
	*collided = false;

	float adjustedOffsetX = entityBounds.AX + moveX;
	float adjustedOffsetY = entityBounds.AY + moveY;

	AABB adjustedEntityBounds = AABB(my_vec2(adjustedOffsetX, adjustedOffsetY));

	for (unsigned int i = 0; i < world->numWalls; i++) {
		AABB wallBounds = AABB(my_vec2((float)world->wallLocations[i].x, (float)world->wallLocations[i].y));

		// First, check adjustEntityBounds for a collision on the wall.
		if (wallBounds.left > adjustedEntityBounds.right) continue;
		if (wallBounds.right < adjustedEntityBounds.left) continue;
		if (wallBounds.top < adjustedEntityBounds.bottom) continue;
		if (wallBounds.bottom > adjustedEntityBounds.top) continue;

		// If we've gotten here, then a collision happened.
		*collided = true;

		// Then, we need to find out what adjustment(s) to make:
		// Depending on which direction the entity was from the wall before movement, we can determine
		// which direction the collision was in and make the adjustment to just move to that wall's 
		// bounds spot.
		if (entityBounds.left >= wallBounds.right) {
			if (entityBounds.bottom >= wallBounds.top || entityBounds.top <= wallBounds.bottom) {}
			else adjustedOffsetX = wallBounds.right;
		}

		if (entityBounds.right <= wallBounds.left) {
			if (entityBounds.bottom >= wallBounds.top || entityBounds.top <= wallBounds.bottom) {

			}
			else {
				adjustedOffsetX = wallBounds.left - 1.0f;
			}
		}

		if (entityBounds.bottom >= wallBounds.top) {
			if (entityBounds.left >= wallBounds.right || entityBounds.right <= wallBounds.left) {

			}
			else {
				adjustedOffsetY = wallBounds.top + 1.0f;
			}
		}

		if (entityBounds.top <= wallBounds.bottom) {
			if (entityBounds.left >= wallBounds.right || entityBounds.right <= wallBounds.left) {

			}
			else {
				adjustedOffsetY = wallBounds.bottom;
			}
		}
	}

	return my_vec2(adjustedOffsetX, adjustedOffsetY);
}

void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed) {
	bool foundBullet = false;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!world->enemyBullets[i].current) {
			world->enemyBullets[i].init(worldOffset,
				my_vec2(dirVec.x, dirVec.y),
				&models.enemyBullet, speed);
			foundBullet = true;
			break;
		}
	}

	if (!foundBullet) printf("Bullet array full! Ah!\n");
}

// UNIFORM SETTING
void refreshProjection() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 5000.0f);

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
		setUniformBool(regularShaderProgramID, lightsCurrent.c_str(), world->lights[i].current);
		
		if (!world->lights[i].current) continue;

		std::string lightsPos		= "lights[" + std::to_string(i) + "].pos";
		std::string lightsAmbient	= "lights[" + std::to_string(i) + "].ambient";
		std::string lightsDiffuse	= "lights[" + std::to_string(i) + "].diffuse";
		std::string lightsSpecular	= "lights[" + std::to_string(i) + "].specular";
		
		setUniform3f(regularShaderProgramID,	lightsPos.c_str(),			world->lights[i].pos);
		setUniform3f(regularShaderProgramID,	lightsAmbient.c_str(),		world->lights[i].ambient);
		setUniform3f(regularShaderProgramID,	lightsDiffuse.c_str(),		world->lights[i].diffuse);
		setUniform3f(regularShaderProgramID,	lightsSpecular.c_str(),		world->lights[i].specular);
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

// GLOBAL STATE STUFF
unsigned int getCurrentLevelIndex() {
	return currentLevelIndex;
}

Level* getCurrentLevel() {
	return &levels[currentLevelIndex];
}

void goBackOneLevel() {
	if (currentLevelIndex == 0) currentLevelIndex = levelCount - 1;
	else currentLevelIndex--;

	loadCurrentLevel();
}

void goForwardOneLevel() {
	currentLevelIndex++;
	if (currentLevelIndex >= levelCount) currentLevelIndex = 0;

	loadCurrentLevel();
}

void goBackOneEnemyType() {
	if	 (enemyTypeSelection == 0) enemyTypeSelection = NUM_ENEMY_TYPES - 1;
	else (enemyTypeSelection)--;
}

void goForwardOneEnemyType() {
	enemyTypeSelection++;
	if ((enemyTypeSelection) >= NUM_ENEMY_TYPES) enemyTypeSelection = 0;
}

void toggleEditorMode() {
	if (editorMode == EDITOR_MODE_ENEMY) editorMode = EDITOR_MODE_WALL;
	else if (editorMode == EDITOR_MODE_WALL) editorMode = EDITOR_MODE_FLOOR;
	else if (editorMode == EDITOR_MODE_FLOOR) editorMode = EDITOR_MODE_FLOOR_FILL;
	else if (editorMode == EDITOR_MODE_FLOOR_FILL) editorMode = EDITOR_MODE_ENEMY;
}

unsigned int getEnemyTypeSelection() {
	return enemyTypeSelection;
}

unsigned int getEditorMode() {
	return editorMode;
}

void loadCurrentLevel() {
	SecureZeroMemory(world, sizeof(world));

	world->init(&models, &eventTextBox, &enemyStrats, &materials);
	world->resetToLevel(&levels[currentLevelIndex]);

	currentInputIndex = 0;

	if (globalMode == MODE_LEVEL_EDIT) {
		camera.initOverhead();
	} else {
		camera.initOnPlayer(world->getPlayerWorldOffset());
	}
}

void deleteCurrentLevel() {
	for (unsigned int i = currentLevelIndex; i < levelCount; i++) {
		levels[i] = levels[i + 1];
		levels[i + 1].initialized = false;
	}
	if (currentLevelIndex == levelCount - 1) {
		currentLevelIndex = 0;
	}
	else {
		currentLevelIndex--;
	}

	levelCount--;
	loadCurrentLevel();
}

void addEnemyToWorld(unsigned int enemyType, my_ivec2 gridCoords) {
	world->addEnemyToWorld(enemyType, gridCoords);
}

void addEnemyToCurrentLevel(int type, my_ivec2 gridCoords) {
	levels[currentLevelIndex].addEnemy(type, gridCoords);
}

void addWallToWorld(my_ivec2 gridCoords) {
	world->addWallToWorld(gridCoords);
}

void addWallToCurrentLevel(my_ivec2 location) {
	unsigned int numWalls = levels[currentLevelIndex].numWalls++;

	levels[currentLevelIndex].wallLocations[numWalls] = location;
}

void addFloorToWorld(my_ivec2 gridCoords) {
	world->addFloorToWorld(gridCoords);
}

void addFloorToCurrentLevel(my_ivec2 location) {
	unsigned int numFloors = levels[currentLevelIndex].numFloors++;

	levels[currentLevelIndex].floorLocations[numFloors] = location;
}

void fillFloor(my_ivec2 gridCoords, Level* level) {
	world->fillFloor(gridCoords);
	world->copyFloorToLevel(level);
}



// TODO: remove ANY entity, not just enemies (walls should count, too)
void removeEntityFromCurrentLevel(my_ivec2 gridCoords) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (levels[currentLevelIndex].enemies[i].gridX == gridCoords.x &&
			levels[currentLevelIndex].enemies[i].gridY == gridCoords.y) {
			levels[currentLevelIndex].removeEnemy(i);
		}
	}
}

void removeEntityFromWorld(my_vec3 worldOffset) {
	world->removeEntityAtOffset(worldOffset);
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
		camera.initOnPlayer(world->getPlayerWorldOffset());

		world->pause();
		globalMode = MODE_GLOBAL_PLAY;
		eventTextBox.addTextToBox("Game Mode: Paused");
		console.flipOut();
	}

	if (commandVector[0] == "freecam") {
		globalMode = MODE_FREE_CAMERA;

		camera.initOnPlayer(world->getPlayerWorldOffset());
		eventTextBox.addTextToBox("Global Mode: Free Camera");
	}

	if (commandVector[0] == "edit") {
		camera.initOverhead();

		globalMode = MODE_LEVEL_EDIT;
		
		loadCurrentLevel();
		world->pause();

		eventTextBox.addTextToBox("Global Mode: Level Edit");
	}

	if (commandVector[0] == "orbit") {
		lightOrbit = !lightOrbit;
		eventTextBox.addTextToBox("Light Orbit: " + std::to_string(lightOrbit));
	}

	if (commandVector[0] == "level count") {
		eventTextBox.addTextToBox("levelCount: " + std::to_string(levelCount));
	}

	if (commandVector[0] == "new") {
		levelCount = addLevel(levels, levelCount);
		currentLevelIndex = levelCount - 1;
		loadCurrentLevel();
	}
}

// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	currentScreenHeight = height;

	currentScreenWidth = (INITIAL_SCREEN_WIDTH * currentScreenHeight) / INITIAL_SCREEN_HEIGHT;

	glViewport((width - currentScreenWidth) / 2, 0, currentScreenWidth, currentScreenHeight);

	console.refresh((float)currentScreenWidth, (float)currentScreenHeight);
	editor.refresh((float)currentScreenWidth, (float)currentScreenHeight);
	
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
	if (globalMode != MODE_LEVEL_EDIT) return;

	if (lastCursorX < 0 || lastCursorX > currentScreenWidth ||
		lastCursorY < 0 || lastCursorY > currentScreenHeight) {
		eventTextBox.addTextToBox("Cursor is off screen, &eventTextBox");
		return;
	}

	// Origin for click coordinates are top-left, y increases downward.
	// Thus, need to get the screen height complement to accurately judge clicks
	// w.r.t. UI elements (which have their origin in bottom left, and y increases upward).
	my_vec2 clickCoords = my_vec2(lastCursorX, glm::abs(lastCursorY - currentScreenHeight));
	
	my_ivec3 gridCoords = cameraCenterToGridCoords();

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		editor.leftClick(clickCoords, gridCoords);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		editor.rightClick(gridCoords);
	}
}
void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	// TODO: Figure out how to lockdown the cursor such that the coordinate args can't go outside the viewport
	//		 Clamping won't work.
	
	if (globalMode == MODE_FREE_CAMERA) {
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
	else if (globalMode == MODE_LEVEL_EDIT) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstMouse = true;

		// TODO: light up moused over grid piece
	}
	
	//else if (mode == MODE_PLAY) {
	//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//	firstMouse = true;
	//}

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
	Mesh floorMesh = Mesh(&regularShaderProgramID, &materials.gold);
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

// LEVEL MANIPULATION
unsigned int addLevel(Level levels[], unsigned int levelCount) {

	levels[levelCount].playerStartX = 100;
	levels[levelCount].playerStartY = 100;

	levels[levelCount].initialized = true;

	return levelCount + 1;
}

unsigned int loadLevelsV2(Level levels[]) {

	const char* fileName = "levels_v2.lev";

	std::ifstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);

	std::stringstream levelStream;
	levelStream << levelFile.rdbuf();
	levelFile.close();

	unsigned int levelCount = 0;
	unsigned int numLevels;

	levelStream >> numLevels;

	while (!levelStream.eof() && levelCount <= MAX_LEVELS && levelCount < numLevels) {
		Level *currentLevel = &levels[levelCount];

		levelStream >> currentLevel->playerStartX;
		levelStream >> currentLevel->playerStartY;

		levelStream >> currentLevel->numWalls;
		for (unsigned int i = 0; i < currentLevel->numWalls; i++) {
			levelStream >> currentLevel->wallLocations[i].x;
			levelStream >> currentLevel->wallLocations[i].y;
		}

		levelStream >> currentLevel->numEnemies;

		for (int i = 0; i < currentLevel->numEnemies; i++) {
			levelStream >> currentLevel->enemies[i].enemyType;
			levelStream >> currentLevel->enemies[i].gridX;
			levelStream >> currentLevel->enemies[i].gridY;
		}

		currentLevel->initialized = true;

		levelCount++;
	}

	return levelCount;
}

void saveAllLevelsV2(Level levels[], unsigned int levelCount) {

	// TODO: logging and error handling
	// TODO: more robust solution --> write to temp file with timestamp, then switch names

	printf("Saving...");

	std::stringstream stringStream;

	stringStream << std::to_string(levelCount) + "\n\n";

	for (int i = 0; i < MAX_LEVELS; i++) {
		Level *currentLevel = &levels[i];
		if (!currentLevel->initialized) break;

		stringStream << std::to_string(currentLevel->playerStartX) + " " + std::to_string(currentLevel->playerStartY) + "\n";

		stringStream << std::to_string(currentLevel->numWalls) + "\n\n";

		for (unsigned int k = 0; k < currentLevel->numWalls; k++) {
			stringStream << std::to_string(currentLevel->wallLocations[k].x) + " " + std::to_string(currentLevel->wallLocations[k].y) + "\n";
		}
		stringStream << "\n";

		stringStream << std::to_string(currentLevel->numEnemies) + "\n";

		for (int j = 0; j < currentLevel->numEnemies; j++) {
			stringStream << std::to_string(currentLevel->enemies[j].enemyType) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridX) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridY) + "\n";
		}

		stringStream << "\n";
	}

	const char* fileName = "levels_v2.lev";
	std::ofstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);
	levelFile.clear();
	levelFile << stringStream.str();
	levelFile.close();

	printf("Saving complete.");
}

// DRAWS, REGULAR
void drawText(Font *font, std::string text, float x, float y, float scale, my_vec3 color) {

	setUniform3f(font->shaderProgramID, "textColor", color);

	// TODO: DON'T FORGET TO WORK OUT THE TEXTURE INFO HERE TOO -- can't just use texture 0 for all fonts
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(font->VAO_ID);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		TextCharacter ch = font->textCharacters[*c];

		float xPos = x + ch.bearing.x * scale;
		float yPos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		float vertices[] = {
			xPos,		yPos + h, 0.0f, 0.0f,
			xPos,		yPos,	  0.0f, 1.0f,
			xPos + w,	yPos,     1.0f, 1.0f,

			xPos,		yPos + h, 0.0f, 0.0f,
			xPos + w,	yPos,	  1.0f, 1.0f,
			xPos + w,	yPos + h, 1.0f, 0.0f,
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, font->VBO_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// this is a lot of draw calls per line of text
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.advance >> 6) * scale; // bit shift changes unit to pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawProspectiveOutline() {
	// turn current center of screen into grid coordinates
	my_ivec3 gridCoords = cameraCenterToGridCoords();

	my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);
	worldOffset.z = 1.05f; // extra for outline visibility

	// draw just an outline in that space
	models.enemy.drawOnlyOutline(worldOffset);

	std::string gridCoordsString = "(" + std::to_string(gridCoords.x) + ", " + std::to_string(gridCoords.y) + ")";
	
	drawText(&arial, gridCoordsString, (float) currentScreenWidth / 2 - 100.0f, (float) currentScreenHeight / 2, 0.5f, my_vec3(0.0f));
}


// UPDATES
void moveLightAroundOrbit(float deltaTime) {
	float radius = 35.0f;
	float speed = 90.0f; // degrees / second
	float degreesMoved = speed * deltaTime;

	float midGridX = (float)20.0f / 2;
	float midGridY = -(float)20.0f / 2;

	float newDegrees = world->lights[0].currentDegrees + degreesMoved;
	if (newDegrees > 360) newDegrees -= 360;

	float newX = glm::cos(glm::radians(newDegrees)) * radius + midGridX;
	float newY = glm::sin(glm::radians(newDegrees)) * radius + midGridY;

	world->lights[0].pos.x = newX;
	world->lights[0].pos.y = newY;

	//world->lightEntity.worldOffset.x = newX;
	//world->lightEntity.worldOffset.y = newY;

	world->lights[0].currentDegrees = newDegrees;
}

int main() {
	world = (WorldState *)VirtualAlloc(0, sizeof(WorldState), MEM_COMMIT, PAGE_READWRITE);
	SecureZeroMemory(world, sizeof(world));
		
	// ------------ INIT STUFF -------------

	// initialization of glfw and glad libraries, window creation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);

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
	arial = Font();
	arial.init("arial.ttf", textShaderProgramID);

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
	createGridFloorAndWallModels();
	createPlayerAndEnemyModels();
    createBulletModel();

	// CONSOLE AND OTHER 2D THINGS
	refreshProjection();
	console.setup(UIShaderProgramID, (float)currentScreenWidth, (float)currentScreenHeight, &arial);
	editor.setup(UIShaderProgramID, &arial, (float)currentScreenWidth, (float)currentScreenHeight);
	fpsBox.x = (float)(currentScreenWidth - 150);
	fpsBox.y = (float)(currentScreenHeight - 50);
	
	// LOAD LEVELS AND SETUP WORLD STATE
	levelCount = loadLevelsV2(levels);
	currentLevelIndex = 0;

	

	loadCurrentLevel();
	
	// FRAME TIMING SETUP
	lastFrameTime = (float)glfwGetTime();

    float deltaTime = 0.0f;
	float timeStep	= deltaTime;

    float targetFrameTime60 = 1.0f / 60.0f;
	float targetFrameTime90 = 1.0f / 90.0f;

	float targetFrameTime = targetFrameTime60;

	// GAMEPAD SETUP
	GLFWgamepadstate gamepadState;	
	
	// FRAGMENT SHADER HOTLOADING SETUP
	WIN32_FILE_ATTRIBUTE_DATA prevFragmentShaderFileData;
	
	WIN32_FILE_ATTRIBUTE_DATA fragmentShaderFileData;
	GetFileAttributesExA(
		"fragmentShader.frag",
		GetFileExInfoStandard,
		&prevFragmentShaderFileData
	);	

	// game loop
	while (!glfwWindowShouldClose(window)) {

		// Checking Fragment Shader for saving
		GetFileAttributesExA(
			"fragmentShader.frag",
			GetFileExInfoStandard,
			&fragmentShaderFileData
		);

		if (fragmentShaderFileData.ftLastWriteTime.dwLowDateTime != prevFragmentShaderFileData.ftLastWriteTime.dwLowDateTime) {
			printf("File updated.\n");
			regularShaderProgramID = createShaderProgram("vertexShader.vert", "fragmentShader.frag");
			
			refreshProjection();
			refreshView();
			prevFragmentShaderFileData = fragmentShaderFileData;
		}

		if(!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepadState)) {	/* TODO: logging */	};

		if (world->numEnemies <= 0 && (world->mode == MODE_PLAY || world->mode == MODE_REPLAY)) {
			loadCurrentLevel();
			currentInputIndex = 0;
			world->mode = MODE_REPLAY;
		}

		// -- INPUT --
		InputRecord currentInputRecord;
		currentInputRecord.gamepadState = gamepadState;
		currentInputRecord.deltaTime = deltaTime;

		processKeyboardInput(window, deltaTime);        
		glfwPollEvents();
		
		// -- UPDATE -- 
		if (globalMode != MODE_LEVEL_EDIT) world->update(currentInputRecord, prevInputRecord, recordedInput, &currentInputIndex);
		if (globalMode == MODE_GLOBAL_PLAY) camera.update(deltaTime, world->getPlayerWorldOffset());
		if (lightOrbit) moveLightAroundOrbit(deltaTime);
		console.update(deltaTime);		

		// -- DRAW --
		// TODO: this should be elsewhere -- it's for fog
		setUniform3f(regularShaderProgramID, "playerPos", world->getPlayerWorldOffset());
		
		refreshView();
		refreshLights();

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.65f, 0.65f, 0.65f, 1.0f);

		glDepthFunc(GL_LESS);
		
		world->draw();
		if (globalMode == MODE_LEVEL_EDIT) drawProspectiveOutline();
		
		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		eventTextBox.drawTextBox(&arial);
		fpsBox.drawTextBox(&arial);
		if(globalMode == MODE_LEVEL_EDIT) editor.draw();

		console.draw();

		// -- FRAME TIMING --
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		
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
		else if (deltaTime > 0.5f) {
			printf("test");
			deltaTime = targetFrameTime;
		}
		else {
			printf("MISSED FRAME! AHH\n"); // TODO: logging
		}

		float frameTime = deltaTime * 1000.0f;

		std::stringstream stream;
		stream << "FrameTime(ms): " << std::setprecision(4) << frameTime;
		drawText(&arial, stream.str(), 0, (float) currentScreenHeight - 30.0f, 0.5f, my_vec3(1.0f));
		
        //if (world->mode == MODE_PAUSED) timeStep = 0.0f;
		globalDeltaTime = deltaTime;

		lastFrameTime = (float)glfwGetTime();

		glfwSwapBuffers(window);
		prevInputRecord = currentInputRecord;
	}

	glfwTerminate();

	return 0;
}
