#include <glad/glad.h>
#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"
#include "shader.h"

glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;

unsigned int w_prevState = GLFW_RELEASE;
unsigned int a_prevState = GLFW_RELEASE;
unsigned int s_prevState = GLFW_RELEASE;
unsigned int d_prevState = GLFW_RELEASE;
unsigned int c_prevState = GLFW_RELEASE;
unsigned int l_prevState = GLFW_RELEASE;
unsigned int spacebar_prevState = GLFW_RELEASE;
unsigned int enter_prevState = GLFW_RELEASE;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

float yaw = -90.0f;
float pitch = 45.0f;

unsigned int cube_VAO_ID;
unsigned int player_VAO_ID;

bool firstMouse = true;
bool freeCamera = false;

struct map {
	bool initialized = false;
	unsigned int grid[GRID_MAP_SIZE_Y][GRID_MAP_SIZE_X];
};

struct character {
	int worldCoordX;
	int worldCoordY;
	int gridCoordX;
	int gridCoordY;

	int shaderProgramID;

	int directionFacing;
	int actionState;

	int hitPoints;
	int strength;
};

struct worldState {
	// This setup will result in a sparse world map. Not a big deal for now, but there is a risk for memory explosion if the size of the possible map expands. (carver - 7-20-20)
	map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

	character player;
	character theOther;
};

worldState world;

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
	}
	c_prevState = c_currentState;
	
	if (freeCamera) {
		const float cameraSpeed = 5.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			cameraPos += cameraSpeed * cameraFront;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			cameraPos -= cameraSpeed * cameraFront;
		}
		// strafe movement
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
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
			if		(world.theOther.actionState == ACTION_STATE_AVOIDANT)	world.theOther.actionState = ACTION_STATE_SEEKING;
			else if (world.theOther.actionState == ACTION_STATE_SEEKING)	world.theOther.actionState = ACTION_STATE_AVOIDANT;
		}
		l_prevState = l_currentState;
	}	
}

void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	if (freeCamera) {
		// what is the unit for offsets here? is it really degrees?
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

		const float sensitivity = 0.1f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}
}

void createCubeVertices() {
	// NOTE: These coords are in local space
	float cubeVertices[] = {
		0.5,  0.0, 0.5,
		0.0,  0.0, 0.5,
		0.5, -0.5, 0.5,
		0.0, -0.5, 0.5,

		0.5,  0.0, 0.0,
		0.0,  0.0, 0.0,
		0.5, -0.5, 0.0,
		0.0, -0.5, 0.0
	};

	unsigned int cubeIndices[] = {
		0,	1,	2,
		1,	3,	2,

		4,	5,	6,
		5,	7,	6,

		0,	1,	4,
		1,	5,	4,

		2,	3,	7,
		2,	7,	6,

		0,	2,	6,
		0,	6,	4,

		1,	3,	7,
		1,	7,	5
	};
	
	glGenVertexArrays(1, &cube_VAO_ID);

	glBindVertexArray(cube_VAO_ID);

	unsigned int cube_VBO_ID;
	glGenBuffers(1, &cube_VBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int cube_EBO_ID;
	glGenBuffers(1, &cube_EBO_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
}

void createPlayerVertices() {
	// NOTE: These coords are in local space
	float playerVertices[] = {
		0.5, 0.5, 0.25,
		0.0, 0.5, 0.25,
		0.5, 0.0, 0.25,
		0.0, 0.0, 0.25,

		0.5, 0.5, 0.0,
		0.0, 0.5, 0.0,
		0.5, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	unsigned int playerIndices[] = {
		0,	1,	2,
		1,	3,	2,

		4,	5,	6,
		5,	7,	6,

		0,	1,	4,
		1,	5,	4,

		2,	3,	7,
		2,	7,	6,

		0,	2,	6,
		0,	6,	4,

		1,	3,	7,
		1,	7,	5
	};

	// TODO: think about pulling the these out into dedicated GPU loading function
	glGenVertexArrays(1, &player_VAO_ID);

	glBindVertexArray(player_VAO_ID);

	unsigned int player_VBO_ID;
	glGenBuffers(1, &player_VBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, player_VBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(playerVertices), playerVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int player_EBO_ID;
	glGenBuffers(1, &player_EBO_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player_EBO_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(playerIndices), playerIndices, GL_STATIC_DRAW);
}

void createSingleGrid(int worldMapX, int worldMapY, int openings, int roomType) {
	
	int newGrid[GRID_MAP_SIZE_X][GRID_MAP_SIZE_Y] = {};

	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			if (row		== 0	|| row		== GRID_MAP_SIZE_X - 1 ||
				column	== 0	|| column	== GRID_MAP_SIZE_Y - 1) {
				newGrid[row][column] = 1;
			}
		}
	}
	
	// NOTE: Relies on even rows/columns to keep exits centered.
	if (openings & LEFT) { // up
		newGrid[GRID_MAP_SIZE_X / 2 - 2][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 - 1][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 + 1][0] = 0;
	}

	if (openings & RIGHT) { // down
		newGrid[GRID_MAP_SIZE_X / 2 - 2][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 - 1][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 + 1][GRID_MAP_SIZE_Y - 1] = 0;	
	}

	if (openings & UP) { // left
		newGrid[0][GRID_MAP_SIZE_Y / 2 - 2] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2 - 1] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2 + 1] = 0;
	}

	if (openings & DOWN) { // right
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 - 2] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 - 1] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 + 1] = 0;
	}

	if (roomType == STORE) {
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 2] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2 + 1] = 1;
	}

	if (roomType == SOMEOTHERTHING) {
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[3][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[4][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[5][GRID_MAP_SIZE_Y / 2 + 0] = 1;
	}

	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			world.allMaps[worldMapX][worldMapY].grid[row][column] = newGrid[row][column];
		}
	}
}

void createAdjacentMaps(int attachedWorldMapX, int attachedWorldMapY, int directionToGetHere) {
	// TODO: fix this directionToGetHere garbage, way too confusing

	int newGridX;
	int newGridY;
	int openings;

	switch (directionToGetHere) {
		case (UP):
			newGridX = attachedWorldMapX;
			newGridY = attachedWorldMapY + 1;
			openings = DOWN;
			break;
		case (DOWN):
			newGridX = attachedWorldMapX;
			newGridY = attachedWorldMapY - 1;
			openings = UP;
			break;
		case (LEFT):
			newGridX = attachedWorldMapX - 1;
			newGridY = attachedWorldMapY;
			openings = RIGHT;
			break;
		case (RIGHT):
			newGridX = attachedWorldMapX + 1;
			newGridY = attachedWorldMapY;
			openings = LEFT;
			break;
	}

	world.allMaps[newGridX][newGridY].initialized = true;
	int roomType = NORMAL;

	// generate down
	if (directionToGetHere != UP) {
		if (rand() % 4 == 2 && newGridY > 0 && !world.allMaps[newGridX][newGridY-1].initialized) {
			openings |= DOWN;
			createAdjacentMaps(newGridX, newGridY, DOWN);
		}
	}

	// generate up
	if (directionToGetHere != DOWN) {
		if (rand() % 10 == 3 && !world.storePlaced) {
			roomType = STORE;
			world.storePlaced = true;
		} else if (rand() % 4 == 2 && newGridY <= WORLD_MAP_SIZE_Y && !world.allMaps[newGridX][newGridY+1].initialized) {
			openings |= UP;
			createAdjacentMaps(newGridX, newGridY, UP);
		}
	}

	// generate right
	if (directionToGetHere != LEFT) {
		if (rand() % 4 == 2 && newGridX <= WORLD_MAP_SIZE_X && !world.allMaps[newGridX+1][newGridY].initialized) {
			openings |= RIGHT;
			createAdjacentMaps(newGridX, newGridY, RIGHT);
		}
	}

	// generate left
	if (directionToGetHere != RIGHT) {
		if (rand() % 10 == 3 && !world.someOtherThingPlaced) {
			roomType = SOMEOTHERTHING;
			world.someOtherThingPlaced = true;
		}
		if (rand() % 4 == 2 && newGridX > 0 && !world.allMaps[newGridX-1][newGridY].initialized) {
			openings |= LEFT;
			createAdjacentMaps(newGridX, newGridY, LEFT);
		}
	}

	createSingleGrid(newGridX, newGridY, openings, roomType);
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

	// setting up Vertex Array Object (VAO)
	unsigned int grid_VAO_ID;
	glGenVertexArrays(1, &grid_VAO_ID);
	glBindVertexArray(grid_VAO_ID);

	// ------------- SHADERS -------------

	unsigned int vertexShaderID = initializeVertexShader("vertexShader.vert");
	unsigned int fragmentShaderID = initializeFragmentShader("fragmentShader.frag");	
	unsigned int shaderProgramID = createShaderProgram(vertexShaderID, fragmentShaderID);

	// start of 3D stuffs
	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgramID, "colorIn");
	
	// creating a view matrix with camera
	// setting up camera
	float midGridX	  =  0.5f * (GRID_MAP_SIZE_X / 2);
	float bottomGridY = -0.5f * (GRID_MAP_SIZE_Y * 2);
	
	cameraPos = glm::vec3(midGridX, bottomGridY, (float)GRID_MAP_SIZE_X / 1.5);
	
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	
	glm::vec3 cameraDirection;

	// TODO: grok this calculation of cameraDirection
	cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraDirection.y = sin(glm::radians(pitch));
	cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(cameraDirection);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection)); // remember: cross product gives you orthongonal vector to both input vectors

	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// aaaaand the projection matrix
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f, 100.0f);	
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// initializing viewport and setting callback for window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	createCubeVertices();
	createPlayerVertices();

	glm::vec3 color1 = glm::vec3(0.4f, 1.0f, 1.0f);
	glm::vec3 color2 = glm::vec3(1.0f, 0.5f, 0.5f);

	glm::vec3 currentColor = color1;

	bool color = true;

	// MAP GENERATION
	createSingleGrid(PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, UP | DOWN | LEFT | RIGHT, false);
	world.allMaps[PLAYER_WORLD_START_X][PLAYER_WORLD_START_Y].initialized = true;

	srand((unsigned int)(glfwGetTime() * 10));
	createAdjacentMaps(PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, UP);
	createAdjacentMaps(PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, DOWN);
	createAdjacentMaps(PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, LEFT);
	createAdjacentMaps(PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, RIGHT);

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

	character* characters[2] = { &world.player, &world.theOther };
	
	lastFrameTime = (float)glfwGetTime();	

	// game loop
	while (!glfwWindowShouldClose(window)) {
		processKeyboardInput(window);
		
		glfwPollEvents();		

		// Move camera
		cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDirection.y = sin(glm::radians(pitch));
		cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(cameraDirection);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Clear color and "z-buffer"
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.7f, 0.3f, 1.0f);	

		// DRAW CURRENT GRID (where the player currently is)
		glBindVertexArray(cube_VAO_ID);
		for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
			float yOffset = -0.5f * row;
			
			for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
				float xOffset = 0.5f * column;

				// position
				glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));
				
				// color
				if (world.allMaps[world.player.worldCoordX][world.player.worldCoordY].grid[row][column] == 0) {
					glUniform3f(colorUniformLocation, color1.r, color1.g, color1.b);
				}
				else {
					glUniform3f(colorUniformLocation, color2.r, color2.g, color2.b);
				}
				
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			}
		}

		// DRAW CHARACTERS
		int numCharacters = sizeof(characters) / sizeof(character*);
		for (int i = 0; i < numCharacters; i++) {
			character currentCharacter = *characters[i];

			if (currentCharacter.worldCoordX != world.player.worldCoordX || currentCharacter.worldCoordY != world.player.worldCoordY) continue;

			float yOffset = -0.5f * currentCharacter.gridCoordY - 0.5f;
			float xOffset = 0.5f * currentCharacter.gridCoordX;

			glBindVertexArray(player_VAO_ID);

			if (i == 0) {
				glUniform3f(colorUniformLocation, 0.0f, 0.45f, 0.03f);
			}
			else {
				glUniform3f(colorUniformLocation, 0.3f, 1.0f, 0.03f);
			}
			
			glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.5f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;	   	 
}