#if !defined(CONSOLE)
#include "textBox.h"
#include "model.h"

struct Console {
	Textbox textbox;

	// TODO: will need to update a lot of this when the screen size changes
	Model background;

	float height = 200.0f;

	float location = currentScreenHeight;
	float destination = currentScreenHeight - height;
	float speed = 10.0f;

	void setup(unsigned int shaderProgramID) {

		float vertices[] = {
			0.0f,				0.0f,	0.0f,
			currentScreenWidth, 0.0f,	0.0f,
			0.0f,				height, 0.0f,
			currentScreenWidth,	height, 0.0f
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 4
		};

		Mesh mesh;
		
		for (int i = 0; i < sizeof(vertices) / sizeof(float); i++) {
			mesh.vertices.push_back(vertices[i]);
		}

		for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++) {
			mesh.indices.push_back(indices[i]);
		}

		mesh.setupVAO();
		mesh.shaderProgramID = shaderProgramID;
		glm::vec3 color = glm::vec3(0.0f, 0.52f, 0.9f);
		mesh.material.diffuse.r = color.x;
		mesh.material.diffuse.g = color.y;
		mesh.material.diffuse.b = color.z;
		mesh.material.ambient.r = color.x;
		mesh.material.ambient.g = color.y;
		mesh.material.ambient.b = color.z;

		background.meshes.push_back(mesh);
	}

	void draw(float deltaTime) {
		float dist = destination - location;
		if (glm::abs(dist) < 1) location = destination;

		float distToMove = dist / 4.0f * deltaTime * speed;
		
		location += distToMove;
		
		textbox.y = location;
		drawTextBox(&textbox);

		// TODO: Fix background drawing...just doesn't work...need another shader specifically for this??? hmmm


		// HACK: this is gross
		unsigned int backgroundShaderID = background.meshes[0].shaderProgramID;

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

		glUseProgram(backgroundShaderID);
		unsigned int projectionLoc = glGetUniformLocation(backgroundShaderID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		background.worldOffset = glm::vec3(0.0f, location, 0.0f);
		background.draw();
	}
};

#define CONSOLE
#endif