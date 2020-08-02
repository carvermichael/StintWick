#if !defined(MODEL)

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <vector>

#include "constants.h"
#include "worldState.h"

struct Material {

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

};

struct Mesh {

	std::vector<float> vertices;
	std::vector<float> texCoords;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	float scaleFactor;

	Material material;

	unsigned int VAO_ID;
	unsigned int verticesVBO_ID;
	unsigned int normalsVBO_ID;
	unsigned int EBO_ID;

	unsigned int shaderProgramID;

	Mesh() {};

	void setUniform3f(unsigned int shaderProgramID, char *uniformName, glm::vec3 vec3) {
		unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
		glUniform3f(location, vec3.x, vec3.y, vec3.z);
	}

	void setUniformMat4(unsigned int shaderProgramID, char *uniformName, glm::mat4 mat4) {
		unsigned int location = glGetUniformLocation(shaderProgramID, uniformName);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat4));
	}

	void draw(glm::vec3 worldOffset, int directionFacing) {
		glUseProgram(shaderProgramID);
		glBindVertexArray(VAO_ID);

		unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
		glm::mat4 current_model = glm::translate(glm::mat4(1.0f), worldOffset);
		
		//current_model = glm::rotate(current_model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
		if (directionFacing == LEFT) {
			current_model = glm::rotate(current_model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (directionFacing == RIGHT) {
			current_model = glm::rotate(current_model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (directionFacing == UP) {
			current_model = glm::rotate(current_model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));

		unsigned int objectAmbientLoc = glGetUniformLocation(shaderProgramID, "objectAmbient");
		glUniform3f(objectAmbientLoc, material.ambient.x, material.ambient.y, material.ambient.z);
		
		unsigned int objectDiffuseLoc = glGetUniformLocation(shaderProgramID, "objectDiffuse");
		glUniform3f(objectDiffuseLoc, material.diffuse.x, material.diffuse.y, material.diffuse.z);

		unsigned int lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
		glUniform3f(lightPosLoc, world.light.pos.x, world.light.pos.y, world.light.pos.z);
		
		unsigned int lightAmbientLoc = glGetUniformLocation(shaderProgramID, "lightAmbient");
		glUniform3f(lightAmbientLoc, world.light.ambient.x, world.light.ambient.y, world.light.ambient.z);
		
		unsigned int lightDiffuseLoc = glGetUniformLocation(shaderProgramID, "lightDiffuse");
		glUniform3f(lightDiffuseLoc, world.light.diffuse.x, world.light.diffuse.y, world.light.diffuse.z);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);	
	}

	void scale(float scale) {
		scaleFactor = scale;
		
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i] *= scaleFactor;
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

	std::vector<Mesh> meshes;
	//std::string directory;

	int directionFacing = DOWN;

	Model() {}

	/*
	Model(std::string path, unsigned int shaderProgramID, float scale)
	{
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_EmbedTextures | aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		//directory = path.substr(0, path.find_last_of('/'));

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh *inputMesh = scene->mMeshes[i];

			Mesh mesh(inputMesh, scene, shaderProgramID, scale);
			this->meshes.push_back(mesh);
		}
	}
	*/

	void draw(glm::vec3 worldOffset) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(worldOffset, directionFacing);
		}
	}

	void scale(float scaleFactor) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].scale(scaleFactor);
		}
	}
};

#define MODEL
#endif