#if !defined(MODEL)

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <vector>

#include "constants.h"

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

struct Material {
	std::string name;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;

	Material() {};

	Material(std::string name, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess) {
		this->name = name;
		
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->shininess = shininess;
	}
};

struct Mesh {

	std::vector<float> vertices;
	std::vector<float> texCoords;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	float scaleFactor;

	Material *material;

	unsigned int VAO_ID;
	unsigned int verticesVBO_ID;
	unsigned int normalsVBO_ID;
	unsigned int EBO_ID;

	unsigned int shaderProgramID;

	Mesh() {};

	void draw(glm::vec3 worldOffset, int directionFacing, Light light) {
		glUseProgram(shaderProgramID);
		glBindVertexArray(VAO_ID);

		glm::mat4 current_model = glm::translate(glm::mat4(1.0f), worldOffset);
		
		setUniformMat4(shaderProgramID, "model", current_model);
		
		setUniform3f(shaderProgramID, "materialAmbient", material->ambient);
		setUniform3f(shaderProgramID, "materialDiffuse", material->diffuse);
		setUniform3f(shaderProgramID, "materialSpecular", material->specular);
		setUniform1f(shaderProgramID, "materialShininess", material->shininess);

		setUniform3f(shaderProgramID, "lightPos", light.pos);
		setUniform3f(shaderProgramID, "lightAmbient", light.ambient);
		setUniform3f(shaderProgramID, "lightDiffuse", light.diffuse);
		setUniform3f(shaderProgramID, "lightSpecular", light.specular);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	void scale(glm::vec3 scale) {
		// NOTE: stride of 6 accounts for normals
		for (int i = 0; i < vertices.size(); i += 6) {
			vertices[i]	  *= scale.x;
			vertices[i+1] *= scale.y;
			vertices[i+2] *= scale.z;
		}

		// Probably a better way to make this transformation in place with openGL.
		reloadToVBOs();
	}

	void reloadToVBOs() {
		glBindBuffer(GL_ARRAY_BUFFER, verticesVBO_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->vertices[0]) * this->vertices.size(), &this->vertices[0]);
	}

	void setupVAO() {
		if (vertices.size() == 0) {
			std::cout << "ERROR: Mesh VAO setup invoked without vertices" << std::endl;
			return;
		}
		
		if (vertices.size() == 0) {
			std::cout << "ERROR: Mesh VAO setup invoked without indices" << std::endl;
			return;
		}

		glGenVertexArrays(1, &this->VAO_ID);
		glBindVertexArray(this->VAO_ID);

		glGenBuffers(1, &verticesVBO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, verticesVBO_ID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), &this->vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // points
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normals
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &EBO_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->indices[0]) * this->indices.size(), &this->indices[0], GL_STATIC_DRAW);
	}
};

struct Model {
	std::string name;

	std::vector<Mesh> meshes;

	Model() {}
	Model(std::string name) {
		this->name = name;
	}
	// TODO TODAY: create constructor (with name)

	void draw(glm::vec3 worldOffset, int directionFacing, Light light) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(worldOffset, directionFacing, light);
		}
	}

	void scale(glm::vec3 scale) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].scale(scale);
		}
	}
};

#define MODEL
#endif