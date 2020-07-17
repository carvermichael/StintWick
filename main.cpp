#include <glad/glad.h>
#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float gridTopLeftX = -0.8f;
float gridTopLeftY = 0.8f;

float sizeOfSide = 0.2f;
float height = sizeOfSide / 2;

unsigned int w_prevState = GLFW_RELEASE;
unsigned int a_prevState = GLFW_RELEASE;
unsigned int s_prevState = GLFW_RELEASE;
unsigned int d_prevState = GLFW_RELEASE;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float lastCursorX = 400;
float lastCursorY = 300;

float yaw = -90.0f;
float pitch = 0;

const int numRows = 8;
const int numColumns = 8;

unsigned int grid_location_VBO_ID;
unsigned int grid_number_vertices = 0;

unsigned int grid_color_VBO_ID;

// Do I have the row/column variables switched? There was something about that w.r.t. world space or local space coords. idk...
// Update: yeah, there's something funky here. A lot of these issues should surface quickly if you make the #rows != #columns.
//		   Pretty sure the issue is just that accessing "rows" and "columns" in a 2D array is flipped from what one might think.
unsigned int currentGrid[numRows][numColumns] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 1, 0, 0,
	1, 0, 0, 0, 0, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 0, 0, 1, 1, 1
};

int playerCoordX = 4;
int playerCoordY = 4;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processKeyboardInput(GLFWwindow *window) {
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	
	const float cameraSpeed = 5.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	//
	//// NOTE: This strategy is not nearly robust enough. It relies on polling the keyboard events. 
	////		 Definite possibility of missing a press or release event here. And frame timing has not
	////		 yet been considered. Probably more reading is required. Still, good enough for exploratory work.
	//int w_currentState = glfwGetKey(window, GLFW_KEY_W);
	//if (w_currentState == GLFW_PRESS && w_prevState == GLFW_RELEASE) {
	//	int prospectiveYCoord = playerCoordY - 1;
	//	if(prospectiveYCoord >= 0 && currentGrid[prospectiveYCoord][playerCoordX] == 0) {
	//		playerCoordY = prospectiveYCoord;
	//	}
	//}
	//w_prevState = w_currentState;

	//int a_currentState = glfwGetKey(window, GLFW_KEY_A);
	//if (a_currentState == GLFW_PRESS && a_prevState == GLFW_RELEASE) {
	//	int prospectiveXCoord = playerCoordX - 1;
	//	if (prospectiveXCoord >= 0 && currentGrid[playerCoordY][prospectiveXCoord] == 0) {
	//		playerCoordX = prospectiveXCoord;
	//	}
	//}
	//a_prevState = a_currentState;

	//int s_currentState = glfwGetKey(window, GLFW_KEY_S);
	//if (s_currentState == GLFW_PRESS && s_prevState == GLFW_RELEASE) {
	//	int prospectiveYCoord = playerCoordY + 1;
	//	if (prospectiveYCoord < numColumns && currentGrid[prospectiveYCoord][playerCoordX] == 0) {
	//		playerCoordY = prospectiveYCoord;
	//	}
	//}
	//s_prevState = s_currentState;

	//int d_currentState = glfwGetKey(window, GLFW_KEY_D);
	//if (d_currentState == GLFW_PRESS && d_prevState == GLFW_RELEASE) {
	//	int prospectiveXCoord = playerCoordX + 1;
	//	if (prospectiveXCoord < numRows && currentGrid[playerCoordY][prospectiveXCoord] == 0) {
	//		playerCoordX = prospectiveXCoord;
	//	}
	//}
	//d_prevState = d_currentState;	
}

void mouseInputCallback(GLFWwindow* window, double xPos, double yPos) {
	// what is the unit for offsets here? is it really degrees?
	float xOffset = (float)(xPos - lastCursorX);
	float yOffset = (float)(lastCursorY - yPos);

	lastCursorX = (float)xPos;
	lastCursorY = (float)yPos;

	const float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;
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

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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

	// setting up vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// loading vertex shader file for compilation
	std::string vertexCode;
	std::ifstream vShaderFile;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	vShaderFile.open("vertexShader.vert");
	std::stringstream vShaderStream;
	vShaderStream << vShaderFile.rdbuf();
	vShaderFile.close();
	vertexCode = vShaderStream.str();
	const char* vShaderCode = vertexCode.c_str();

	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// setting up the fragment shaders
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string fragmentCode;
	std::ifstream fShaderFile;
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.open("fragmentShader.frag");
	std::stringstream fShaderStream;
	fShaderStream << fShaderFile.rdbuf();
	fShaderFile.close();
	fragmentCode = fShaderStream.str();
	const char* fShaderCode = fragmentCode.c_str();

	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);

	success = 0;
	memset(infoLog, 0, sizeof(infoLog));
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// initializing shader program/pipeline, attaching compiled shaders, initialization, linking, and shader cleanup
	unsigned int shaderProgram_ID;
	shaderProgram_ID = glCreateProgram();

	glAttachShader(shaderProgram_ID, vertexShader);
	glAttachShader(shaderProgram_ID, fragmentShader);
	glLinkProgram(shaderProgram_ID);

	success = 0;
	memset(infoLog, 0, sizeof(infoLog));
	glGetProgramiv(shaderProgram_ID, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderProgram_ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glUseProgram(shaderProgram_ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// start of 3D stuffs
	// creating a model matrix
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 identityModel = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	unsigned int modelLoc = glGetUniformLocation(shaderProgram_ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// creating a view matrix with camera
	// setting up camera

	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection)); // remember: cross product gives you orthongonal vector to both input vectors

	//glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	unsigned int viewLoc = glGetUniformLocation(shaderProgram_ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// aaaaand the projection matrix
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	unsigned int projectionLoc = glGetUniformLocation(shaderProgram_ID, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// initializing viewport and setting callback for window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseInputCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, 800, 600);

	glEnable(GL_DEPTH_TEST);

	// NOTE: These coords are in local space
	float cubeVertices[] = {
		0.5, 0.5, 0.5,
		0.0, 0.5, 0.5,
		0.5, 0.0, 0.5,
		0.0, 0.0, 0.5,

		0.5, 0.5, 0.0,
		0.0, 0.5, 0.0,
		0.5, 0.0, 0.0,
		0.0, 0.0, 0.0,

		// TODO: remove these when we've verified it's working with just ebo indices of 0-7
		0.5, 0.5, 0.5, // 0
		0.0, 0.5, 0.5, // 1
		0.5, 0.5, 0.0, // 4
		0.0, 0.5, 0.0, // 5

		0.5, 0.0, 0.5, // 2
		0.0, 0.0, 0.5, // 3
		0.5, 0.0, 0.0, // 6
		0.0, 0.0, 0.0, // 7
		
		0.5, 0.5, 0.5, // 0
		0.5, 0.0, 0.5, // 2
		0.5, 0.5, 0.0, // 3
		0.5, 0.0, 0.0, // 6

		0.0, 0.5, 0.5, // 1
		0.0, 0.0, 0.5, // 3
		0.0, 0.5, 0.0, // 5
		0.0, 0.0, 0.0  // 7 
	};

	unsigned int cubeIndices[] = {
		0,	1,	2,
		1,	3,	2,

		4,	5,	6,
		5,	7,	6,

		0,	1,	4,
		1,	5,	4,

		2,	3,	6,
		2,	7,	6,

		0,	2,	3,
		0,	6,	3,

		1,	3,	5,
		1,	7,	5
	};

	unsigned int cube_VAO_ID;
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

	glm::vec3 color1 = glm::vec3(0.4f, 1.0f, 1.0f);
	glm::vec3 color2 = glm::vec3(1.0f, 0.5f, 0.5f);

	unsigned int colorUniformLocation = glGetUniformLocation(shaderProgram_ID, "colorIn");
	
	glm::vec3 currentColor = color1;

	bool color = true;

	lastFrameTime = (float)glfwGetTime();

	// game loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		processKeyboardInput(window);

		cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDirection.y = sin(glm::radians(pitch));
		cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(cameraDirection);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.7f, 0.3f, 1.0f);
		
		glUniform3f(colorUniformLocation, color1.r, color1.g, color1.b);		

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;	   	 
}