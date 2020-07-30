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

#include "mesh.h"
#include "camera.h"
#include "worldState.h"
#include "constants.h"
#include "shader.h"
#include "worldGeneration.h"

void addTextToBox(std::string newText);

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

unsigned int cube_VAO_ID;
unsigned int player_VAO_ID;

bool firstMouse = true;
bool freeCamera = false;

worldState world = {};
std::vector<WorldObject> worldObjects;

unsigned int shaderProgramID;

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

struct TextCharacter {
	unsigned int textureID;
	glm::ivec2	 size;
	glm::ivec2	 bearing;
	unsigned int advance;
};

std::map<char, TextCharacter> textCharacters;
unsigned int textVAOID, textVBOID;
unsigned int textShaderProgramID;

// TODO: try doing this without std::string
void drawText(unsigned int shaderProgramID, std::string text, float x, float y, float scale, glm::vec3 color) {

	glUseProgram(shaderProgramID);
	unsigned int textColorLoc = glGetUniformLocation(shaderProgramID, "textColor");
	glUniform3f(textColorLoc, color.x, color.y, color.z);

	glm::mat4 textProjection = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(textProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAOID);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		TextCharacter ch = textCharacters[*c];

		float xPos = x + ch.bearing.x * scale;
		float yPos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		float vertices[6][4] = {
			xPos,		yPos + h, 0.0f, 0.0f,
			xPos,		yPos,	  0.0f, 1.0f,
			xPos + w,	yPos,     1.0f, 1.0f,

			xPos,		yPos + h, 0.0f, 0.0f,
			xPos + w,	yPos,	  1.0f, 1.0f,
			xPos + w,	yPos + h, 1.0f, 0.0f,
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, textVBOID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.advance >> 6) * scale; // bit shift changes unit to pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#define LIMIT_LINES 4

struct TextBox {
	std::string lines[LIMIT_LINES];
	unsigned int startingLineIndex = 0;
};

TextBox textBox = {};

void drawTextBox() {
	unsigned int numLinesRendered = 0;
	unsigned int currentLineIndex = textBox.startingLineIndex;
	float x = 0.0f, y = 0.0f;

	while (numLinesRendered < LIMIT_LINES) {
		drawText(textShaderProgramID, textBox.lines[currentLineIndex], x, y, 0.4f, glm::vec3(1.0f, 0.5f, 0.89f));

		currentLineIndex++;
		if (currentLineIndex >= LIMIT_LINES) {
			currentLineIndex = 0;
		}

		y += 20.0f;

		numLinesRendered++;
	}
}

void addTextToBox(std::string newText) {
	textBox.lines[textBox.startingLineIndex] = newText;

	textBox.startingLineIndex++;
	if (textBox.startingLineIndex >= LIMIT_LINES) {
		textBox.startingLineIndex = 0;
	}
}

void drawGrid() {
	glUseProgram(shaderProgramID);
	glBindVertexArray(cube_VAO_ID);

	glm::vec3 color1 = glm::vec3(0.4f, 1.0f, 1.0f);
	glm::vec3 color2 = glm::vec3(1.0f, 0.5f, 0.5f);

	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgramID, "colorIn");

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
}

void drawCharacters(Character *characters[]) {
	int numCharacters = 2;

	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgramID, "colorIn");

	for (int i = 0; i < numCharacters; i++) {
		Character currentCharacter = *characters[i];

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
}

void generateObjectsFromDungeonScene() {
	Assimp::Importer importer;
	const aiScene *dungeonScene = importer.ReadFile("dungeonScene.fbx", aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType); // TODO: figure out which processing flags you might want on load here (using example's default for now)

	if (!dungeonScene) {
		std::cout << importer.GetErrorString() << std::endl;
	}

	aiNode *rootNode = dungeonScene->mRootNode;

	for (unsigned int i = 0; i < rootNode->mNumChildren; i++) {
		WorldObject worldObject;

		aiNode *child = rootNode->mChildren[i];

		for (unsigned int j = 0; j < child->mNumMeshes; j++) {
			Mesh mesh;
			aiMesh *inputMesh = dungeonScene->mMeshes[child->mMeshes[j]];

			createMesh(&mesh, inputMesh, dungeonScene);

			worldObject.meshes.push_back(mesh);
		}

		worldObjects.push_back(worldObject);
	}
}

void drawWorldObjects() {

	float xOffset = 0.0f;
	float yOffset = 0.0f;

	glUseProgram(shaderProgramID);

	glm::vec3 color1 = glm::vec3(0.4f, 1.0f, 1.0f);
	glm::vec3 color2 = glm::vec3(1.0f, 0.5f, 0.5f);

	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgramID, "colorIn");

	glUniform3f(colorUniformLocation, color1.r, color1.g, color1.b);

	for (int i = 0; i < worldObjects.size(); i++) {
		WorldObject *worldObject = &worldObjects[i];

		yOffset += -0.5f;
		xOffset += 0.5f;

		glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));

		for (int j = 0; j < worldObject->meshes.size(); j++) {
			Mesh *mesh = &worldObject->meshes[j];

			glBindVertexArray(mesh->VAO_ID);

			glDrawElements(GL_TRIANGLES, mesh->indices.size() / 3, GL_UNSIGNED_INT, 0);
		}
	}
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
	shaderProgramID = createShaderProgram(vertexShaderID, fragmentShaderID);

	// start of 3D stuffs
	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgramID, "colorIn");
	
	world.camera.initialize();
	
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

	Character* characters[2] = { &world.player, &world.theOther };
	
	lastFrameTime = (float)glfwGetTime();

	// TEXT RENDERING
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	FT_Face face;
	if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	// 0 for width means dynamically adjust based on height
	FT_Set_Pixel_Sizes(face, 0, 48);

	// generate a texture for each ASCII character
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		}

		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		TextCharacter textCharacter = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};

		textCharacters.insert(std::pair<char, TextCharacter>(c, textCharacter));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	//// TEXT SHADER PROGRAM
	unsigned int textVertexShaderID = initializeVertexShader("textVertexShader.vert");
	unsigned int textFragShaderID   = initializeFragmentShader("textFragmentShader.frag");
	textShaderProgramID = createShaderProgram(textVertexShaderID, textFragShaderID);

	// need alpha blending for text transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// reserving data for text on gpu
	glGenVertexArrays(1, &textVAOID);
	glGenBuffers(1, &textVBOID);
	glBindVertexArray(textVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, textVBOID);

	/*
		Note the NULL:
		So far, in other calls like this, we give a pointer to the data at the same time we describe the data.
		However, this time we're going to create each text character's vertices on the fly (as they each have their
		own spacial needs. So, this first Data call just describes the data, and the subData call later sends along
		the vertices. -- carver (7-27-20)
	*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	generateObjectsFromDungeonScene();

	// game loop	
	while (!glfwWindowShouldClose(window)) {
		glUseProgram(shaderProgramID);
		
		processKeyboardInput(window);
		
		glfwPollEvents();

		glm::mat4 view = world.camera.generateView();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// Clear color and z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		drawWorldObjects();

		//drawGrid();
		//drawCharacters(characters);
		drawTextBox();

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;	   	 
}