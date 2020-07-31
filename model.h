#if !defined(MODEL)

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <vector>

// no textures with the new low poly models, just material colors
/*
struct Texture {

	unsigned int ID;
	std::string fileName;
	std::string type;

	Texture(std::string inputFileName, std::string type) {
		int textureWidth, textureHeight, textureNRChannels;

		unsigned char *textureData = stbi_load(inputFileName.c_str(), &textureWidth, &textureHeight, &textureNRChannels, 0);

		if (!textureData) {
			std::cout << "Failed to load texture: " + fileName << std::endl;
			return;
		}
		else {
			std::cout << "Successfully loaded texture: " + fileName << std::endl;
		}

		GLenum format;
		if (textureNRChannels == 1)
			format = GL_RED;
		else if (textureNRChannels == 3)
			format = GL_RGB;
		else if (textureNRChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &this->ID);

		glBindTexture(GL_TEXTURE_2D, this->ID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(textureData);
	}
};
*/

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

	// no textures with the new low poly models, just material colors
	//std::vector<Texture> textures;

	Material material;

	unsigned int VAO_ID;
	unsigned int shaderProgramID;

	Mesh() {};

	Mesh(aiMesh *inputMesh, const aiScene *scene, unsigned int shaderProgramID) {

		this->shaderProgramID = shaderProgramID;

		// Question: Is loading the vertices, texCoords, and normals in these three loops faster or slower than a single loop? My intuition says faster,
		//			 'cause the data is accessed more sequentially than in the case of a single loop.

		const unsigned int numVertices = inputMesh->mNumVertices; // Does this make it easier on the compiler?

		// vertices
		for (unsigned int i = 0; i < numVertices; i++) {
			aiVector3D vector3D = inputMesh->mVertices[i];

			this->vertices.push_back(vector3D.x);
			this->vertices.push_back(vector3D.y);
			this->vertices.push_back(vector3D.z);
		}

		// no textures with the new low poly models, just material colors
		// texCoords
		/*
		for (unsigned int i = 0; i < numVertices; i++) {
			aiVector3D vector3D = inputMesh->mTextureCoords[0][i]; // can have 8 sets of texCoords...only need the first

			this->texCoords.push_back(vector3D.x);
			this->texCoords.push_back(vector3D.y);
		}
		*/

		// normals
		for (unsigned int i = 0; i < numVertices; i++) {
			aiVector3D vector3D = inputMesh->mNormals[i];

			this->normals.push_back(vector3D.x);
			this->normals.push_back(vector3D.y);
			this->normals.push_back(vector3D.z);
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
		/*
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
		*/

		setupVAO();
	}

	void draw(glm::vec3 worldOffset) {
		glUseProgram(shaderProgramID);
		glBindVertexArray(VAO_ID);

		unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
		glm::mat4 current_model = glm::translate(glm::mat4(1.0f), worldOffset);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(current_model));

		// TODO: set material uniforms correctly and do the lighting thing
		unsigned int colorInLoc = glGetUniformLocation(shaderProgramID, "colorIn");
		glUniform3f(colorInLoc, material.diffuse.x, material.diffuse.y, material.diffuse.z);

		// TODO: get the number of triangles to draw correctly
		glDrawElements(GL_TRIANGLES, 320, GL_UNSIGNED_INT, 0);
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

		unsigned int VerticesVBO_ID;
		glGenBuffers(1, &VerticesVBO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, VerticesVBO_ID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), &this->vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// no textures with the new low poly models, just material colors
		/*
		unsigned int TexCoordsVBO_ID;
		glGenBuffers(1, &TexCoordsVBO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, TexCoordsVBO_ID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->texCoords[0]) * this->texCoords.size(), &this->texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		*/

		// TODO: guarantee normals are created for all objects
		if (normals.size() > 0) {
			unsigned int NormalsVBO_ID;
			glGenBuffers(1, &NormalsVBO_ID);
			glBindBuffer(GL_ARRAY_BUFFER, NormalsVBO_ID);
			glBufferData(GL_ARRAY_BUFFER, sizeof(this->normals[0]) * this->normals.size(), &this->normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(2);
		}

		unsigned int EBO_ID;
		glGenBuffers(1, &EBO_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->indices[0]) * this->indices.size(), &this->indices[0], GL_STATIC_DRAW);
	}
};

struct Model {

	std::vector<Mesh> meshes;
	//std::string directory;

	Model() {}

	Model(std::string path, unsigned int shaderProgramID)
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

			Mesh mesh(inputMesh, scene, shaderProgramID);
			this->meshes.push_back(mesh);
		}
	}

	void draw(glm::vec3 worldOffset) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(worldOffset);
		}
	}
};

#define MODEL
#endif