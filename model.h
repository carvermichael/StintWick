#if !defined(MODEL)

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <vector>

#include "constants.h"

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
	
	// Not using right now. Creating own geometry for the time being.
	
	/*
	Mesh(aiMesh *inputMesh, const aiScene *scene, unsigned int shaderProgramID, float inputScale) {

		this->shaderProgramID = shaderProgramID;

		// Question: Is loading the vertices, texCoords, and normals in these three loops faster or slower than a single loop? My intuition says faster,
		//			 'cause the data is accessed more sequentially than in the case of a single loop.

		const unsigned int numVertices = inputMesh->mNumVertices; // Does this make it easier on the compiler?

		scaleFactor = inputScale;

		// vertices
		for (unsigned int i = 0; i < numVertices; i++) {
			aiVector3D vector3D = inputMesh->mVertices[i];
			
			this->vertices.push_back(vector3D.x * scaleFactor);
			this->vertices.push_back(vector3D.y * scaleFactor);
			this->vertices.push_back(vector3D.z * scaleFactor);
		}

		
		// normals
		for (unsigned int i = 0; i < numVertices; i++) {
			aiVector3D vector3D = inputMesh->mNormals[i];
			
			this->normals.push_back(vector3D.x * scaleFactor);
			this->normals.push_back(vector3D.y * scaleFactor);
			this->normals.push_back(vector3D.z * scaleFactor);
		}

		// indices
		for (unsigned int i = 0; i < inputMesh->mNumFaces; i++) {
			aiFace face = inputMesh->mFaces[i];

			// We did set the flag to triangulate the indices, but just in case...
			if (face.mNumIndices != 3) {
				continue;
			}

			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				this->indices.push_back(face.mIndices[j]);
			}
		}

		// TODO: tangents and bitangents, when you know what those are about

		aiMaterial *mat = scene->mMaterials[inputMesh->mMaterialIndex];
		
		// ambient
		aiColor3D ambientColor;
		mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);

		material.ambient.x = ambientColor.r;
		material.ambient.y = ambientColor.g;
		material.ambient.z = ambientColor.b;
		
		// diffuse
		aiColor3D diffuseColor;
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

		material.diffuse.x = diffuseColor.r;
		material.diffuse.y = diffuseColor.g;
		material.diffuse.z = diffuseColor.b;

		// specular
		aiColor3D specularColor;
		mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

		material.specular.x = specularColor.r;
		material.specular.y = specularColor.g;
		material.specular.z = specularColor.b;

		// no textures with the new low poly models, just material colors
		
		unsigned int ambientTextureCount = mat->GetTextureCount(aiTextureType_AMBIENT);
		for (unsigned int i = 0; i < ambientTextureCount; i++) {
			aiString path;

			mat->GetTexture(aiTextureType_AMBIENT, i, &path);

			Texture texture(path.C_Str(), "ambient");
			this->textures.push_back(texture);
		}

		unsigned int diffuseTextureCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
		for (unsigned int i = 0; i < diffuseTextureCount; i++) {
			aiString path;

			mat->GetTexture(aiTextureType_DIFFUSE, i, &path);

			Texture texture(path.C_Str(), "diffuse");
			this->textures.push_back(texture);
		}

		unsigned int specularTextureCount = mat->GetTextureCount(aiTextureType_SPECULAR);
		for (unsigned int i = 0; i < specularTextureCount; i++) {
			aiString path;

			mat->GetTexture(aiTextureType_SPECULAR, i, &path);

			Texture texture(path.C_Str(), "specular");
			this->textures.push_back(texture);
		}
		

		setupVAO();
	}
	
	*/
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

		//unsigned int lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
		//glUniform3f(lightPosLoc, light.pos.x, light.pos.y, light.pos.z);

		//unsigned int lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
		//glUniform3f(lightColorLoc, light.diffuse.x, light.diffuse.y, light.diffuse.z);

		//unsigned int objectColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
		//glUniform3f(objectColorLoc, material.diffuse.x, material.diffuse.y, material.diffuse.z);

		 //TODO: set material uniforms correctly and do the lighting thing
		unsigned int objectAmbientLoc = glGetUniformLocation(shaderProgramID, "objectAmbient");
		glUniform3f(objectAmbientLoc, material.ambient.x, material.ambient.y, material.ambient.z);
		
		unsigned int objectDiffuseLoc = glGetUniformLocation(shaderProgramID, "objectDiffuse");
		glUniform3f(objectDiffuseLoc, material.diffuse.x, material.diffuse.y, material.diffuse.z);

		unsigned int lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
		glUniform3f(lightPosLoc, light.pos.x, light.pos.y, light.pos.z);
		
		unsigned int lightAmbientLoc = glGetUniformLocation(shaderProgramID, "lightAmbient");
		glUniform3f(lightAmbientLoc, light.ambient.x, light.ambient.y, light.ambient.z);
		
		unsigned int lightDiffuseLoc = glGetUniformLocation(shaderProgramID, "lightDiffuse");
		glUniform3f(lightDiffuseLoc, light.diffuse.x, light.diffuse.y, light.diffuse.z);

		// TODO: get the number of triangles to draw correctly
		glDrawElements(GL_TRIANGLES, 3200, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
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