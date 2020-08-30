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
#include <vector>
#include <stdlib.h>
#include <map>

void createParticleEmitter(glm::vec3 newPos);
void processConsoleCommand(std::string command);
void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value);
void setUniform3f(unsigned int shaderProgramID, const char *uniformName, glm::vec3 vec3);
void setUniform4f(unsigned int shaderProgramID, const char *uniformName, glm::vec4 vec4);
void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4);

#include "constants.h"

unsigned int currentScreenHeight	= INITIAL_SCREEN_HEIGHT;
unsigned int currentScreenWidth		= INITIAL_SCREEN_WIDTH;

#include "model.h"
Models models;
Materials materials;

#include "camera.h"

#include "worldState.h"

#include "shader.h"
#include "textBox.h"
#include "console.h"
#include "shapeData.h"
#include "levels.h"

unsigned int regularShaderProgramID;
unsigned int lightShaderProgramID;
unsigned int UIShaderProgramID;
unsigned int textShaderProgramID;

unsigned int mode = MODE_PLAY;

// Note: Can't add an extra param to key callback. Could use the window's user pointer, but I'm lazy. (carver - 8-20-20)
float globalDeltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

bool firstMouse = true;
bool freeCamera = false;
int timeStepDenom = 1;

Textbox eventTextBox = {};
Textbox fpsBox = {};

glm::mat4 projection;

bool lightOrbit = false;
bool guidingGrid = false;

Console console;
WorldState world;

#include "controls.h"

struct Follow : EnemyStrat {

	void update(Entity *entity, Player *player, float deltaTime) {
		glm::vec3 dirVec = glm::normalize(player->worldOffset - entity->worldOffset);
		glm::vec3 newWorldOffset = entity->worldOffset + (dirVec * entity->speed * deltaTime);

		entity->updateWorldOffset(newWorldOffset.x, newWorldOffset.y);
	}
};

struct Shoot : EnemyStrat {

	void update(Entity *entity, Player *player, float deltaTime) {
		glm::vec3 dirVec = glm::normalize(player->worldOffset - entity->worldOffset);

		entity->timeSinceLastShot += deltaTime;
		if (entity->timeSinceLastShot < entity->timeBetweenShots) return;

		bool foundBullet = false;
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (!world.enemyBullets[i].current) {
				world.enemyBullets[i].init(entity->worldOffset,
					glm::vec2(dirVec.x, dirVec.y),
					&models.enemyBullet, entity->shotSpeed);
				foundBullet = true;
				break;
			}
		}

		if (!foundBullet) printf("Bullet array full! Ah!\n");
		else entity->timeSinceLastShot = 0.0f;
	}
};

struct EnemyStrats {

	Follow follow;
	Shoot shoot;

};

EnemyStrats enemyStrats;

void refreshProjection() {
	projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

	setUniformMat4(regularShaderProgramID, "projection", projection);
	setUniformMat4(lightShaderProgramID, "projection", projection);

	glm::mat4 UIProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	
	setUniformMat4(textShaderProgramID, "projection", UIProjection);
	setUniformMat4(UIShaderProgramID, "projection", UIProjection);
}

void refreshView() {
	glm::mat4 view;
	view = world.camera.generateView();

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

	console.refresh();

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
}

void processJoystickInput(float deltaTime) {
    if(mode != MODE_PLAY) return; 
    
    GLFWgamepadstate state;

    if(glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        moveWithController(state, deltaTime);
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

void processConsoleCommand(std::string command) {
	std::vector<std::string> commandVector = splitString(command, ' ');
	
	if (commandVector[0] == "play") {
		//world.camera.initializeForGrid();

		mode = MODE_PLAY;
		addTextToBox("Mode: Play", &eventTextBox);
	}

	if (commandVector[0] == "freecam") {
		mode = MODE_FREE_CAMERA;

		world.camera.initializeForGrid(world.gridSizeX, world.gridSizeY);
		addTextToBox("Mode: Free Camera", &eventTextBox);
	}

	if (commandVector[0] == "edit") {
		world.camera.initializeOverhead(world.gridSizeX, world.gridSizeY);

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

		setMaterial(commandVector[1], commandVector[2], &materials, &models, &console);
	}
}

void setUniform1f(unsigned int shaderProgramID, const char *uniformName, float value) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform1f(location, value);
}

void setUniform3f(unsigned int shaderProgramID, const char *uniformName, glm::vec3 vec3) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform3f(location, vec3.x, vec3.y, vec3.z);
}

void setUniform4f(unsigned int shaderProgramID, const char *uniformName, glm::vec4 vec4) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniform4f(location, vec4.x, vec4.y, vec4.z, vec4.w);
}

void setUniformMat4(unsigned int shaderProgramID, const char *uniformName, glm::mat4 mat4) {
	glUseProgram(shaderProgramID);
	unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat4));
}

// TODO: clean up model creation
void createBulletModel() {
	Mesh bulletMesh;
	Mesh enemyBulletMesh;
	Mesh bulletPartMesh;

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		bulletMesh.vertices.push_back(cubeVertices[i]);
		enemyBulletMesh.vertices.push_back(cubeVertices[i]);
		bulletPartMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
		bulletMesh.indices.push_back(cubeIndices[i]);
		enemyBulletMesh.indices.push_back(cubeIndices[i]);
		bulletPartMesh.indices.push_back(cubeIndices[i]);
	}

	for (int i = 0; i < sizeof(cubeOutlineIndices) / sizeof(unsigned int); i++) {
		bulletMesh.outlineIndices.push_back(cubeOutlineIndices[i]);		
		enemyBulletMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
		bulletPartMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
	}
	
	bulletMesh.setupVAO();
	bulletMesh.shaderProgramID = regularShaderProgramID;
	bulletMesh.material = &materials.grey;
	bulletMesh.drawOutline = true;

	models.bullet.name = std::string("bullet");
	models.bullet.meshes.push_back(bulletMesh);
	models.bullet.scale(glm::vec3(0.5f));

	enemyBulletMesh.setupVAO();
	enemyBulletMesh.shaderProgramID = regularShaderProgramID;
	enemyBulletMesh.material = &materials.gold;
	enemyBulletMesh.drawOutline = true;

	models.enemyBullet.name = std::string("enemyBullet");
	models.enemyBullet.meshes.push_back(enemyBulletMesh);
	models.enemyBullet.scale(glm::vec3(0.5f));

	bulletPartMesh.setupVAO();
	bulletPartMesh.shaderProgramID = regularShaderProgramID;
	bulletPartMesh.material = &materials.grey;
	bulletPartMesh.drawOutline = true;

	models.bulletPart.name = std::string("bulletPart");
	models.bulletPart.meshes.push_back(bulletPartMesh);
	models.bulletPart.scale(glm::vec3(0.15f));
}

void createGridFloorAndWallModels() {
	Mesh floorMesh;	
	Mesh wallTopMesh;
	Mesh wallLeftMesh;

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		floorMesh.vertices.push_back(cubeVertices[i]);
		wallTopMesh.vertices.push_back(cubeVertices[i]);
		wallLeftMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
		floorMesh.indices.push_back(cubeIndices[i]);
		wallTopMesh.indices.push_back(cubeIndices[i]);
		wallLeftMesh.indices.push_back(cubeIndices[i]);
	}

	for (int i = 0; i < sizeof(cubeOutlineIndices) / sizeof(unsigned int); i++) {
		floorMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
		wallTopMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
		wallLeftMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
	}
	
	floorMesh.setupVAO();
	floorMesh.shaderProgramID = regularShaderProgramID;
	floorMesh.material = &materials.emerald;
	floorMesh.drawOutline = false;
	
	wallTopMesh.setupVAO();
	wallTopMesh.shaderProgramID = regularShaderProgramID;
	wallTopMesh.material = &materials.gold;
	wallTopMesh.drawOutline = false;

	wallLeftMesh.setupVAO();
	wallLeftMesh.shaderProgramID = regularShaderProgramID;
	wallLeftMesh.material = &materials.gold;
	wallLeftMesh.drawOutline = false;

	models.floorModel.name = std::string("floor");
	models.floorModel.meshes.push_back(floorMesh);
	
	models.wallLeftModel.name = std::string("wall");
	models.wallLeftModel.meshes.push_back(wallLeftMesh);
	
	models.wallTopModel.name = std::string("wall");
	models.wallTopModel.meshes.push_back(wallTopMesh);	
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

	for (int i = 0; i < sizeof(cubeOutlineIndices) / sizeof(unsigned int); i++) {
		playerMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
	}

	playerMesh.setupVAO();
	playerMesh.shaderProgramID = regularShaderProgramID;
	playerMesh.material = &materials.chrome;
	playerMesh.drawOutline = true;

	models.player.name = std::string("player");
	models.player.meshes.push_back(playerMesh);
	//models.player.scale(glm::vec3(0.9f, 0.9f, 0.25f));

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		enemyMesh.vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(unsigned int); i++) {
		enemyMesh.indices.push_back(cubeIndices[i]);
	}

	for (int i = 0; i < sizeof(cubeOutlineIndices) / sizeof(unsigned int); i++) {
		enemyMesh.outlineIndices.push_back(cubeOutlineIndices[i]);
	}

	enemyMesh.setupVAO();
	enemyMesh.shaderProgramID = regularShaderProgramID;
	enemyMesh.material = &materials.ruby;
	playerMesh.drawOutline = true;

	models.enemy.name = std::string("enemy");
	models.enemy.meshes.push_back(enemyMesh);
	models.enemy.scale(glm::vec3(1.0f, 1.0f, 0.5f));
}

void drawGrid() {
	glm::vec3 floorWorldOffset = glm::vec3(1.0f, -1.0f, 0.0f);
	models.floorModel.draw(floorWorldOffset);

	// left
	models.wallLeftModel.draw(glm::vec3(0.0f, 0.0f, 0.0f));
	// top
	models.wallTopModel.draw(glm::vec3(1.0f, 0.0f, 0.0f));
	
	// right
	models.wallLeftModel.draw(glm::vec3((float)world.gridSizeX - 1.0f, 0.0f, 0.0f));
	// bottom
	models.wallTopModel.draw(glm::vec3(1.0f, -(float)world.gridSizeY + 1.0f, 0.0f));
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

	float lowerZ = 0.0f;
	float upperZ = 5.0f;
	float zStep	 = 1.0f;

	float lowerY = -100.f;
	float upperY =  100.f;
	float yStep  =  1.0f;

	float lowerX = -100.f;
	float upperX =	100.f;
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

	glm::mat4 currentModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
	setUniformMat4(regularShaderProgramID, "model", currentModel);

	setUniform3f(regularShaderProgramID, "objectDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	setUniform3f(regularShaderProgramID, "objectAmbient", glm::vec3(1.0f, 1.0f, 1.0f));

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
        if(world.enemies[i].current) world.enemies[i].draw();
    }
}

void updateEnemies(float deltaTime) {
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (world.enemies[i].current) world.enemies[i].update(&world.player, deltaTime);
	}
}

void checkBulletsForWallCollisions() {

    // TODO: bug: collision with north wall is off (I would guess by 0.5f) --> bullet detects hit too early
    //            Is BY off by 0.5f?
    for(int i = 0; i < MAX_BULLETS; i++) {
        if(!world.playerBullets[i].current) continue;
        if(world.playerBullets[i].worldOffset.x < world.wallBounds.AX ||
           world.playerBullets[i].worldOffset.x > world.wallBounds.BX ||
           world.playerBullets[i].worldOffset.y > world.wallBounds.AY ||
           world.playerBullets[i].worldOffset.y < world.wallBounds.BY) {
            world.playerBullets[i].current = false;
			createParticleEmitter(glm::vec3(world.playerBullets[i].worldOffset.x,
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
        }
    }
}

void createParticleEmitter(glm::vec3 newPos) {
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

void initializeWorld(unsigned int level[]) {
	world.init(level[0], level[1]);

	world.camera.initializeForGrid(world.gridSizeX, world.gridSizeY);

	world.player.init(gridCoordsToWorldOffset(glm::ivec3(level[2], level[3], 1)), &models.player);

	unsigned int numOfEnemies = level[4];
	for (unsigned int i = 5, j = 0; i < 5 + numOfEnemies * 3; i += 3, j++) {

		unsigned int enemyType = level[i];
		unsigned int gridX = level[i+1];
		unsigned int gridY = level[i+2];

		EnemyStrat *strat = &enemyStrats.shoot;
		if (enemyType == 1) {
			strat = &enemyStrats.shoot;
		} else if (enemyType == 2) {
			strat = &enemyStrats.follow;
		}

		world.enemies[j].init(gridCoordsToWorldOffset(glm::ivec3(gridX, gridY, 1)), &models.enemy, strat);
	}
	
	models.floorModel.scale(glm::vec3((float)world.gridSizeX - 2.0f, (-(float)world.gridSizeY) + 2.0f, 1.0f));
	models.wallLeftModel.scale(glm::vec3(1.0f, -1.0f * world.gridSizeY, 2.0f));
	models.wallTopModel.scale(glm::vec3((float)world.gridSizeX - 2.0f, -1.0f, 2.0f));
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	guidingGridSetup();
	createGridFloorAndWallModels();
	createPlayerAndEnemyModels();
    createBulletModel();

	console.setup(UIShaderProgramID);
	fpsBox.x = (float)(currentScreenWidth - 150);
	fpsBox.y = (float)(currentScreenHeight - 50);

	initializeWorld(level_1);

	lastFrameTime = (float)glfwGetTime();

    float deltaTime = 0.0f;
	float timeStep	= deltaTime;

    float targetFrameTime = 1.0f / 60.0f;    

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
		if(lightOrbit) moveLightAroundOrbit(timeStep);

        refreshLight();
        updateBullets(timeStep);
        checkBulletsForWallCollisions();
        checkBulletsForEnemyCollisions();
		updateEnemies(timeStep);
		updateParticleEmitters(timeStep);
		
		drawGrid();
	    world.player.draw();
        drawEnemies();
        drawBullets();
		drawParticleEmitters();
			
		// test cube
		//models.floorModel.draw(glm::vec3(15.0f, -10.0f, -2.0f));

		// UI Elements
		glDepthFunc(GL_ALWAYS); // always buffer overwrite - in order of draw calls
		console.draw(deltaTime, &arial);
		drawTextBox(&eventTextBox, &arial);
		drawTextBox(&fpsBox, &arial);
		
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;	
		//printf("DeltaTime: %f ", deltaTime);
		
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

		// TODO: fix this hack --> should be a simple ACTUAL textbox that you can put this into. The "textBox" then is a group of those.
		//addTextToBoxAtLine("FPS: " + std::to_string(fps), 0, &fpsBox); 
		//printf("FrameTime(ms): %f ", frameTime); // put these in a textbox
		//printf("FPS: %f \n", 1.0f / deltaTime);

        // TODO: Use some other variable to communicate a timestep.
        timeStep = deltaTime / timeStepDenom;
        globalDeltaTime = deltaTime;

		lastFrameTime = (float)glfwGetTime();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}
