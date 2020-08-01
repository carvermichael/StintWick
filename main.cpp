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

struct Light {

	glm::vec3 pos;
	glm::vec3 ambient;
	glm::vec3 diffuse;

};

Light light;

#include "model.h"
#include "camera.h"
#include "worldState.h"
#include "constants.h"
#include "shader.h"
#include "worldGeneration.h"
#include "textBox.h"

// TODO: combine previous state
unsigned int w_prevState = GLFW_RELEASE;
unsigned int a_prevState = GLFW_RELEASE;
unsigned int s_prevState = GLFW_RELEASE;
unsigned int d_prevState = GLFW_RELEASE;
unsigned int c_prevState = GLFW_RELEASE;
unsigned int l_prevState = GLFW_RELEASE;
unsigned int m_prevState = GLFW_RELEASE;
unsigned int spacebar_prevState = GLFW_RELEASE;
unsigned int enter_prevState = GLFW_RELEASE;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

bool firstMouse = true;
bool freeCamera = false;

WorldState world = {};

Model playerModel;
Model theOtherModel;

Model floorModel;
Model lightCube;

Model wallModel;
Model decorativeWallModel;
Model wallCoverModel;

unsigned int regularShaderProgramID;
unsigned int lightShaderProgramID;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
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
	world.player.directionFacing = direction;

	if (direction & UP) {
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

	if (direction & DOWN) {
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

	if (direction & LEFT) {
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

	if (direction & RIGHT) {
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

void processKeyboardInput(GLFWwindow *window) {
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	int c_currentState = glfwGetKey(window, GLFW_KEY_C);
	if (c_currentState == GLFW_PRESS && c_prevState == GLFW_RELEASE) {
		freeCamera = !freeCamera;
		if (freeCamera) {
			addTextToBox("Camera Free");
		} else {
			addTextToBox("Camera Locked");
		}
	}
	c_prevState = c_currentState;
	
	if (freeCamera) {
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
	}
	else {
		// NOTE: This strategy is not nearly robust enough. It relies on polling the keyboard events. 
		//		 Definite possibility of missing a press or release event here. And frame timing has not
		//		 yet been considered. Probably more reading is required. Still, good enough for exploratory work.
		int w_currentState = glfwGetKey(window, GLFW_KEY_W);
		if (w_currentState == GLFW_PRESS && w_prevState == GLFW_RELEASE) {
			moveTheOther();
			movePlayer(UP);
		}
		w_prevState = w_currentState;

		int a_currentState = glfwGetKey(window, GLFW_KEY_A);
		if (a_currentState == GLFW_PRESS && a_prevState == GLFW_RELEASE) {
			moveTheOther();
			movePlayer(LEFT);
		}
		a_prevState = a_currentState;

		int s_currentState = glfwGetKey(window, GLFW_KEY_S);
		if (s_currentState == GLFW_PRESS && s_prevState == GLFW_RELEASE) {
			moveTheOther();
			movePlayer(DOWN);
		}
		s_prevState = s_currentState;

		int d_currentState = glfwGetKey(window, GLFW_KEY_D);
		if (d_currentState == GLFW_PRESS && d_prevState == GLFW_RELEASE) {
			moveTheOther();
			movePlayer(RIGHT);
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
				addTextToBox("AI Set To AVOIDANT");
				world.theOther.actionState = ACTION_STATE_SEEKING;
			}
			else if (world.theOther.actionState == ACTION_STATE_SEEKING) {
				addTextToBox("AI Set To SEEKING");
				world.theOther.actionState = ACTION_STATE_AVOIDANT;
			}
		}
		l_prevState = l_currentState;
	}	
}

void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	if (freeCamera) {
		if (firstMouse)
		{
			lastCursorX = (float)xPos;
			lastCursorY = (float)yPos;
			firstMouse = false;
		}

		float xOffset = (float)(xPos - lastCursorX);
		float yOffset = (float)(lastCursorY - yPos);

		lastCursorX = (float)xPos;
		lastCursorY = (float)yPos;

		world.camera.adjustYawAndPitch(xOffset, yOffset);		
	}
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

	playerModel.meshes.push_back(playerMesh);

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
				wallModel.draw(offset);
				
				/*
				if (row == GRID_MAP_SIZE_X - 1) {
					wallModel.directionFacing = UP;
					wallModel.draw(offset);
				}
				if (row == 0) {
					wallModel.directionFacing = DOWN;
					wallModel.draw(offset);
				}
				if (column == GRID_MAP_SIZE_Y - 1) {
					wallModel.directionFacing = LEFT;
					wallModel.draw(offset);
				}
				if (column == 0) {
					wallModel.directionFacing = RIGHT;
					wallModel.draw(offset);
				}
				*/
				
			} else {
				floorModel.draw(offset);
			}
		}
	}
}

void drawThePlayer() {
	float zOffset = 0.5f;
	float yOffset = -0.5f * world.player.gridCoordY - 0.5f;
	float xOffset = 0.5f * world.player.gridCoordX;

	playerModel.draw(glm::vec3(xOffset, yOffset, zOffset));
}

void drawTheOther() {
	if (world.theOther.worldCoordX != world.player.worldCoordX || world.theOther.worldCoordY != world.player.worldCoordY) return;
	
	float zOffset = 0.5f;
	float yOffset = -0.5f * world.theOther.gridCoordY - 0.5f;
	float xOffset = 0.5f * world.theOther.gridCoordX;

	theOtherModel.draw(glm::vec3(xOffset, yOffset, zOffset));
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

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GridGame1", NULL, NULL);
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
	{
		unsigned int vertexShaderID = initializeVertexShader("vertexShader.vert");
		unsigned int fragmentShaderID = initializeFragmentShader("fragmentShader.frag");
		regularShaderProgramID = createShaderProgram(vertexShaderID, fragmentShaderID);
	}
	
	{
		unsigned int lightVertexShaderID = initializeVertexShader("lightVertexShader.vert");
		unsigned int lightFragmentShaderID = initializeFragmentShader("lightFragmentShader.frag");
		lightShaderProgramID = createShaderProgram(lightVertexShaderID, lightFragmentShaderID);
	}

	
	// start of 3D stuffs
	glUseProgram(regularShaderProgramID);
	unsigned int viewLoc = glGetUniformLocation(regularShaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(regularShaderProgramID, "projection");
	
	// aaaaand the projection matrix
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f, 100.0f);	
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	

	glUseProgram(lightShaderProgramID);
	unsigned int lightViewLoc = glGetUniformLocation(lightShaderProgramID, "view");
	unsigned int lightProjectionLoc = glGetUniformLocation(lightShaderProgramID, "projection");
	glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// initializing viewport and setting callback for window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	createLightCube();
	light.pos = glm::vec3(-2.0f, -5.0f, 4.0f);
	light.ambient = glm::vec3(1.0f);
	light.diffuse = glm::vec3(1.0f);
	
	createGridFloorAndWallModels();
	createPlayerAndTheOtherModels();

	// set seed and generate map
	unsigned int seed = (unsigned int)time(NULL); // seconds since Jan 1, 2000
	srand(seed);
	addTextToBox("seed: " + std::to_string(seed));
	
	generateWorldMap(&world);

	// CHARACTER INITIALIZATION
	world.player.worldCoordX = PLAYER_WORLD_START_X;
	world.player.worldCoordY = PLAYER_WORLD_START_Y;
	world.player.gridCoordX = PLAYER_GRID_START_X;
	world.player.gridCoordY = PLAYER_GRID_START_Y;
	world.player.strength = 1;
	world.player.hitPoints = 20;

	world.theOther.worldCoordX = PLAYER_WORLD_START_X;
	world.theOther.worldCoordY = PLAYER_WORLD_START_Y;
	world.theOther.gridCoordX = 1;
	world.theOther.gridCoordY = 2;
	world.theOther.hitPoints = 3;
	world.theOther.strength = 1;

	lastFrameTime = (float)glfwGetTime();

	initializeTextBox();

	// need alpha blending for text transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	world.camera.initializeForGrid();
	//world.camera.initialize();

	// game loop
	while (!glfwWindowShouldClose(window)) {
		processKeyboardInput(window);
		
		glfwPollEvents();
		
		glUseProgram(regularShaderProgramID);
		// TODO: This needs to be done somewhere else. This will break when there's more than one shader for world objects.
		//		 Per each draw invocation? Or when the view changes, put it in all the shaders? ehhh...
		glm::mat4 view = world.camera.generateView();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glUseProgram(lightShaderProgramID);
		// TODO: This needs to be done somewhere else. This will break when there's more than one shader for world objects.
		//		 Per each draw invocation? Or when the view changes, put it in all the shaders? ehhh...
		glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		
		lightCube.draw(glm::vec3(-2.0f, -5.0f, 4.0f));

		drawGrid();
		drawThePlayer();
		drawTheOther();

		drawTextBox();

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}