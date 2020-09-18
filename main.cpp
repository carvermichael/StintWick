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

// Dumb forward declarations...
void createParticleEmitter(my_vec3 newPos);
void processConsoleCommand(std::string command);
void loadCurrentLevel();
void goBackOneLevel();
void goForwardOneLevel();
void goBackOneEnemyType();
void goForwardOneEnemyType();
unsigned int getCurrentLevel();
unsigned int getEnemySelection();
void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed);
void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value);
void setUniform3f(unsigned int shaderProgramID, const char *uniformName, my_vec3 my_vec3);
void setUniform4f(unsigned int shaderProgramID, const char *uniformName, my_vec4 my_vec4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, my_mat4 mat4);

#include "model.h"
#include "camera.h"
#include "worldState.h"
#include "shader.h"
#include "textBox.h"
#include "console.h"
#include "editorUI.h"
#include "levels.h"

// TODO: Get all of this on the heap
	// Random Global State
	unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;
	unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
	unsigned int mode = MODE_PLAY;
	float globalDeltaTime = 0.0f;
	float lastFrameTime = 0.0f;
	float lastCursorX = 400;
	float lastCursorY = 300;
	unsigned int levelCount;
	unsigned int currentLevel = 0;
	unsigned int currentEnemyTypeSelection = 0;
	bool firstMouse = true;
	int timeStepDenom = 1;
	bool pause = true;
	bool lightOrbit = false;
	bool guidingGrid = false;

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

	EditorUI editorUI;
	Console console;
	WorldState world;
	Level levels[MAX_LEVELS];
	#include "shapeData.h"

	Textbox eventTextBox = {};
	Textbox fpsBox = {};

#include "controls.h"

void createBullet(my_vec3 worldOffset, my_vec3 dirVec, float speed) {
	bool foundBullet = false;
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!world.enemyBullets[i].current) {
			world.enemyBullets[i].init(worldOffset,
				my_vec2(dirVec.x, dirVec.y),
				&models.enemyBullet, speed);
			foundBullet = true;
			break;
		}
	}

	if (!foundBullet) printf("Bullet array full! Ah!\n");
}

void refreshProjection() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

	//printGLMMat4(projection);

	setUniformMat4(regularShaderProgramID, "projection", projection);
	setUniformMat4(lightShaderProgramID, "projection", projection);

	glm::mat4 UIProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	
	setUniformMat4(textShaderProgramID, "projection", UIProjection);
	setUniformMat4(UIShaderProgramID, "projection", UIProjection);
}

void refreshView() {
	
	//glm::mat4 view = world.camera.generateView();

	my_mat4 view = world.camera.generateMyView();

	setUniformMat4(regularShaderProgramID, "view", view);
	setUniformMat4(lightShaderProgramID, "view", view);

	setUniform3f(regularShaderProgramID, "viewPos", world.camera.position);
	setUniform3f(lightShaderProgramID, "viewPos", world.camera.position);
}

void refreshLight() {

	setUniform3f(regularShaderProgramID, "lightPos", world.light.pos);
	setUniform3f(regularShaderProgramID, "lightAmbient", world.light.ambient);
	setUniform3f(regularShaderProgramID, "lightDiffuse", world.light.diffuse);
	setUniform3f(regularShaderProgramID, "lightSpecular", world.light.specular);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	currentScreenHeight = height;
	
	currentScreenWidth = (INITIAL_SCREEN_WIDTH * currentScreenHeight) / INITIAL_SCREEN_HEIGHT;
	
	glViewport((width - currentScreenWidth) / 2, 0, currentScreenWidth, currentScreenHeight);

	console.refresh((float)currentScreenWidth, (float)currentScreenHeight);
	editorUI.refresh((float)currentScreenWidth, (float)currentScreenHeight);

    refreshProjection();
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (console.isOut) {
		console.addInput((char) codepoint);
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
	if(ctlFunc) ctlFunc(action, key, globalDeltaTime);
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

void processKeyboardInput(GLFWwindow *window, float deltaTime) {
	// NOTE: Using the callback for free camera movement is super choppy,
	//		 Cause it's the only thing that involves holding down keys?
	if (mode == MODE_FREE_CAMERA) {
		const float cameraSpeed = 25.0f * deltaTime;
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
	else if (mode == MODE_LEVEL_EDIT) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			world.camera.moveUpOne();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			world.camera.moveDownOne();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			world.camera.moveLeftOne();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			world.camera.moveRightOne();
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			world.camera.moveForward(deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			world.camera.moveBack(deltaTime);
		}
	}
}

void processJoystickInput(float deltaTime) {
    if(mode != MODE_PLAY) return; 
    
    GLFWgamepadstate state;

    if(glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        moveWithController(state, deltaTime);
    }
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
	if (world.numEnemies >= MAX_ENEMIES) {
		printf("ERROR: Max enemies reached.\n");
		addTextToBox("ERROR: Max enemies reached.", &eventTextBox);
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

	world.enemies[world.numEnemies].init(gridCoordsToWorldOffset(my_ivec3(gridCoords.x, gridCoords.y, 1)), &models.enemy, mat, strat);
	world.numEnemies++;
}

my_ivec3 cameraCenterToGridCoords() {
	my_vec3 startingPos = world.camera.position;
	my_vec3 dirVec = world.camera.front;

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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (mode != MODE_LEVEL_EDIT) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// check in window
		if (lastCursorX < 0 || lastCursorX > currentScreenWidth ||
			lastCursorY < 0 || lastCursorY > currentScreenHeight) {
			addTextToBox("Cursor is off screen, &eventTextBox", &eventTextBox);
			return;
		}

		// Origin for click coordinates are top-left, y increases downward.
		// Thus, need to get the screen height complement to accurately judge clicks
		// w.r.t. UI elements (which have their origin in bottom left, and y increases upward).

		// check in editor box
		if (mode == MODE_LEVEL_EDIT) {
			if (editorUI.click(my_vec2(lastCursorX, glm::abs(lastCursorY - currentScreenHeight)))) return;
		}
				
		// place enemy
		// TODO: Raycast from click coords instead of camera center.
		// Slight hack to get a working way to place enemies before groking a raycast from click coords. (carver - 9-09-2020)
		my_ivec3 gridCoords = cameraCenterToGridCoords();

		// TODO: maybe find a better way to keep world and level in sync
		//			- may end up with a staging level struct at some point
		addEnemyToWorld(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
		addEnemyToLevel(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		// get grid coords
		my_ivec3 gridCoords = cameraCenterToGridCoords();
		my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);

		// remove target enemy from world
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (world.enemies[i].current) {
				if (worldOffset.x == world.enemies[i].worldOffset.x &&
					worldOffset.y == world.enemies[i].worldOffset.y) {
					world.enemies[i].current = false;
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

		// TODO: light up moused over grid piece

	} else if (mode == MODE_PLAY) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstMouse = true;
	}

	lastCursorX = (float)xPos;
	lastCursorY = (float)yPos;
}

void processConsoleCommand(std::string command) {
	std::vector<std::string> commandVector = splitString(command, ' ');
	
	if (commandVector[0] == "play") {
		world.camera.initForGrid(world.gridSizeX, world.gridSizeY);

		mode = MODE_PLAY;
		addTextToBox("Mode: Play", &eventTextBox);
	}

	if (commandVector[0] == "freecam") {
		mode = MODE_FREE_CAMERA;

		world.camera.initForGrid(world.gridSizeX, world.gridSizeY);
		addTextToBox("Mode: Free Camera", &eventTextBox);
	}

	if (commandVector[0] == "edit") {
		world.camera.initOverhead(world.gridSizeX, world.gridSizeY);

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

	if (commandVector[0] == "level count") {
		addTextToBox("levelCount: " + std::to_string(levelCount), &eventTextBox);
	}

	if (commandVector[0] == "newLevel") {

		if (commandVector.size() < 3) {
			addTextToBox("error: missing args gridSizeX and gridSizeY", &eventTextBox);
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
			addTextToBox("error: missing args for material and model", &eventTextBox);
			return;
		}

		setMaterial(commandVector[1], commandVector[2], &materials, &models, &console);
	}
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

void createBulletModel() {
	Mesh bulletMesh = Mesh(regularShaderProgramID, &materials.grey);
	models.bullet.name = std::string("bullet");
	models.bullet.meshes.push_back(bulletMesh);
	models.bullet.scale(my_vec3(0.5f));

	Mesh enemyBulletMesh = Mesh(regularShaderProgramID, &materials.gold);
	models.enemyBullet.name = std::string("enemyBullet");
	models.enemyBullet.meshes.push_back(enemyBulletMesh);
	models.enemyBullet.scale(my_vec3(0.5f));

	Mesh bulletPartMesh = Mesh(regularShaderProgramID, &materials.grey);
	models.bulletPart.name = std::string("bulletPart");
	models.bulletPart.meshes.push_back(bulletPartMesh);
	models.bulletPart.scale(my_vec3(0.15f));
}

void createGridFloorAndWallModels() {
	Mesh floorMesh = Mesh(regularShaderProgramID, &materials.emerald);
	models.floorModel.name = std::string("floor");
	models.floorModel.meshes.push_back(floorMesh);
	
	Mesh wallLeftMesh = Mesh(regularShaderProgramID, &materials.gold);
	models.wallLeftModel.name = std::string("wall");
	models.wallLeftModel.meshes.push_back(wallLeftMesh);
	
	Mesh wallTopMesh = Mesh(regularShaderProgramID, &materials.gold);
	models.wallTopModel.name = std::string("wall");
	models.wallTopModel.meshes.push_back(wallTopMesh);	
}

void createPlayerAndEnemyModels() {
	Mesh playerMesh = Mesh(regularShaderProgramID, &materials.chrome);
	models.player.name = std::string("player");
	models.player.meshes.push_back(playerMesh);
	
	Mesh enemyMesh = Mesh(regularShaderProgramID, &materials.ruby);
	models.enemy.name = std::string("enemy");
	models.enemy.meshes.push_back(enemyMesh);
	models.enemy.scale(my_vec3(1.0f, 1.0f, 0.5f));
}

void drawGrid() {
	my_vec3 floorWorldOffset = my_vec3(1.0f, -1.0f, 0.0f);
	models.floorModel.draw(floorWorldOffset);

	// left
	models.wallLeftModel.draw(my_vec3(0.0f, 0.0f, 0.0f));
	// top
	models.wallTopModel.draw(my_vec3(1.0f, 0.0f, 0.0f));
	
	// right
	models.wallLeftModel.draw(my_vec3((float)world.gridSizeX - 1.0f, 0.0f, 0.0f));
	// bottom
	models.wallTopModel.draw(my_vec3(1.0f, -(float)world.gridSizeY + 1.0f, 0.0f));
}

void moveLightAroundOrbit(float deltaTime) {
	float radius = 35.0f;
	float speed = 90.0f; // degrees / second
	float degreesMoved = speed * deltaTime;

	float midGridX = (float) world.gridSizeX / 2;
	float midGridY = -(float) world.gridSizeY / 2;

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

void updateBullets(float deltaTime) {
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(world.playerBullets[i].current) {
            Bullet *bullet = &world.playerBullets[i];
            bullet->updateWorldOffset(bullet->worldOffset.x + bullet->direction.x * bullet->speed * deltaTime, 
                                      bullet->worldOffset.y + bullet->direction.y * bullet->speed * deltaTime);
        }

		if (world.enemyBullets[i].current) {
			Bullet *bullet = &world.enemyBullets[i];
			bullet->updateWorldOffset(bullet->worldOffset.x + bullet->direction.x * bullet->speed * deltaTime,
				bullet->worldOffset.y + bullet->direction.y * bullet->speed * deltaTime);
		}
    }
}

void drawBullets() {
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(world.playerBullets[i].current) world.playerBullets[i].draw();
    }

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (world.enemyBullets[i].current) world.enemyBullets[i].draw();
	}
}

void drawEnemies() {
    for(int i = 0; i < MAX_ENEMIES; i++) {
		float outlineFactor = 1.0f;
		if(world.enemies[i].current) world.enemies[i].draw();
    }
}

void updateEnemies(float deltaTime) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (world.enemies[i].current) world.enemies[i].update(&world.player, deltaTime);
	}
}

void checkBulletsForWallCollisions() {

    for(int i = 0; i < MAX_BULLETS; i++) {
        if(!world.playerBullets[i].current) continue;
        if(world.playerBullets[i].worldOffset.x < world.wallBounds.AX ||
           world.playerBullets[i].worldOffset.x > world.wallBounds.BX ||
           world.playerBullets[i].worldOffset.y > world.wallBounds.AY ||
           world.playerBullets[i].worldOffset.y < world.wallBounds.BY) {
            world.playerBullets[i].current = false;
			createParticleEmitter(my_vec3(world.playerBullets[i].worldOffset.x,
											world.playerBullets[i].worldOffset.y,
											1.5f));
        }
    }

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!world.enemyBullets[i].current) continue;
		if (world.enemyBullets[i].worldOffset.x < world.wallBounds.AX ||
			world.enemyBullets[i].worldOffset.x > world.wallBounds.BX ||
			world.enemyBullets[i].worldOffset.y > world.wallBounds.AY ||
			world.enemyBullets[i].worldOffset.y < world.wallBounds.BY) {
			world.enemyBullets[i].current = false;
		}
	}
}

void checkBulletsForEnemyCollisions() {

    for(int i = 0; i < MAX_BULLETS; i++) {

        if(!world.playerBullets[i].current) continue;
        Bullet *bullet = &world.playerBullets[i];

        for(int j = 0; j < MAX_ENEMIES; j++) {

            if(!world.enemies[j].current) continue;
            Enemy *enemy = &world.enemies[j];

            if(bullet->bounds.left   > enemy->bounds.right)  continue;
            if(bullet->bounds.right  < enemy->bounds.left)   continue;
            if(bullet->bounds.top    < enemy->bounds.bottom) continue;
            if(bullet->bounds.bottom > enemy->bounds.top)    continue;

            enemy->current = false;
			bullet->current = false;
			world.numEnemies--;
        }
    }
}

void createParticleEmitter(my_vec3 newPos) {
	bool foundEmitter = false;
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (!world.particleEmitters[i].current) {
			world.particleEmitters[i].init(newPos, &models.bulletPart);
			foundEmitter = true;
			break;
		}
	}

	if (!foundEmitter) printf("ParticleEmitter array full! Ah!\n");
}

void updateParticleEmitters(float deltaTime) {
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (world.particleEmitters[i].current) {
			world.particleEmitters[i].update(deltaTime);
		}
	}
}

void drawParticleEmitters() {
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		if (world.particleEmitters[i].current) {
			world.particleEmitters[i].draw();
		}
	}
}

void loadCurrentLevel() {
	Level *level = &levels[currentLevel];

	world.init(level->sizeX, level->sizeY);

	world.camera.initForGrid(world.gridSizeX, world.gridSizeY);

	world.player.init(gridCoordsToWorldOffset(my_ivec3(level->playerStartX, level->playerStartY, 1)), &models.player);

	for (int i = 0; i < MAX_ENEMIES; i++) {
		world.enemies[i].current = false;
	}

	world.numEnemies = 0;

	unsigned int numOfEnemies = level->numEnemies;
	for (unsigned int i = 0; i < numOfEnemies; i++) {

		unsigned int enemyType = level->enemies[i].enemyType;
		unsigned int gridX = level->enemies[i].gridX;
		unsigned int gridY = level->enemies[i].gridY;

		addEnemyToWorld(enemyType, my_ivec2(gridX, gridY));
	}

	// reset camera
	if (mode == MODE_LEVEL_EDIT) {
		world.camera.initOverhead(world.gridSizeX, world.gridSizeY);
	}
	else {
		world.camera.initForGrid(world.gridSizeX, world.gridSizeY);
	}	

	// clear particles
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		world.particleEmitters[i].current = false;
	}

	for (int i = 0; i < MAX_BULLETS; i++) {
		world.playerBullets[i].current = false;
		world.enemyBullets[i].current = false;
	}

	models.floorModel.rescale(my_vec3((float)world.gridSizeX - 2.0f, (-(float)world.gridSizeY) + 2.0f, 1.0f));
	models.wallLeftModel.rescale(my_vec3(1.0f, -1.0f * world.gridSizeY, 2.0f));
	models.wallTopModel.rescale(my_vec3((float)world.gridSizeX - 2.0f, -1.0f, 2.0f));
}

void drawProspectiveOutline() {
	// turn current center of screen into grid coordinates
	my_ivec3 gridCoords = cameraCenterToGridCoords();
	
	my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);
	worldOffset.z = 1.05f; // extra for outline visibility

	// draw just an outline in that space
	models.enemy.drawOnlyOutline(worldOffset);
}

void drawPlayerBlinking(float deltaTime) {

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

	glfwWindowHint(GLFW_SAMPLES, 4); // for MSAA, takes 4 samples per pixel, bufferSize *= 4
	GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "GridGame1", NULL, NULL);
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
	
	guidingGridSetup();
	createGridFloorAndWallModels();
	createPlayerAndEnemyModels();
    createBulletModel();

	console.setup(UIShaderProgramID, (float)currentScreenWidth, (float)currentScreenHeight);
	fpsBox.x = (float)(currentScreenWidth - 150);
	fpsBox.y = (float)(currentScreenHeight - 50);

	levelCount = loadLevels(levels);
	currentLevel = 0;

	loadCurrentLevel();
	
	lastFrameTime = (float)glfwGetTime();

    float deltaTime = 0.0f;
	float timeStep	= deltaTime;

    float targetFrameTime = 1.0f / 60.0f;

	editorUI.setup(UIShaderProgramID, &arial, (float)currentScreenWidth, (float)currentScreenHeight);

	world.player.blinking = true;

	// game loop
	while (!glfwWindowShouldClose(window)) {
		processKeyboardInput(window, deltaTime);
        processJoystickInput(timeStep);
		
		glfwPollEvents();

		refreshView();

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);

		glDepthFunc(GL_LESS);
		if(guidingGrid)	drawGuidingGrid();
		if(lightOrbit) moveLightAroundOrbit(deltaTime);

        refreshLight();
        updateBullets(timeStep);
        checkBulletsForWallCollisions();
        checkBulletsForEnemyCollisions();
		updateEnemies(timeStep);
		updateParticleEmitters(timeStep);
		
		drawGrid();
	    world.player.draw(deltaTime);
        drawEnemies();
        drawBullets();
		drawParticleEmitters();
		if (mode == MODE_LEVEL_EDIT) drawProspectiveOutline();
				
		if (world.numEnemies <= 0) {
			pause = true;
			goForwardOneLevel();
		}
	
		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		console.draw(deltaTime, &arial);
		drawTextBox(&eventTextBox, &arial);
		drawTextBox(&fpsBox, &arial);
		if(mode == MODE_LEVEL_EDIT) editorUI.draw();
		
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;	
		
        if(deltaTime < targetFrameTime) {
			int timeToSleepMS = (int) (1000.0f * (targetFrameTime - deltaTime));
            
			if (timeToSleepMS > 1) {
				Sleep(timeToSleepMS);
			}

			while (deltaTime < targetFrameTime) {
				deltaTime = (float)glfwGetTime() - lastFrameTime;
			}
		}
		else {
			printf("MISSED FRAME! AHH\n"); // TODO: logging
		}

		float frameTime = deltaTime * 1000.0f;

		std::stringstream stream;
		stream << "FrameTime(ms): " << std::setprecision(4) << frameTime;
		drawText(&arial, stream.str(), 0, (float) currentScreenHeight - 30.0f, 0.5f, my_vec3(1.0f));
		
        timeStep = deltaTime / timeStepDenom;
		if (pause) timeStep = 0.0f;
		globalDeltaTime = deltaTime;

		lastFrameTime = (float)glfwGetTime();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}
