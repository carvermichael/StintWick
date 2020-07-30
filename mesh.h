#if !defined(MESH)

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <vector>

struct Mesh {

	std::vector<float> vertices;
	std::vector<float> texCoords;
	std::vector<float> normals; // (probably won't need until we do lighting, but might as well get 'em in here anyway)
	std::vector<unsigned int> indices;

	std::string textureFileName; // maybe want a pointer to the texture (or something like that)?
	std::vector <std::string> ambientTextureFileNames;
	std::vector <std::string> diffuseTextureFileNames;
	std::vector <std::string> specularTextureFileNames;

	unsigned int VAO_ID;
};

void createMesh(Mesh *mesh, aiMesh *inputMesh, aiScene const *scene) {

	// Question: Is loading the vertices, texCoords, and normals in these three loops faster or slower than a single loop? My intuition says faster,
	//			 'cause the data is accessed more sequentially than in the case of a single loop.

	const unsigned int numVertices = inputMesh->mNumVertices; // Does this make it easier on the compiler?

	// vertices
	for (unsigned int i = 0; i < numVertices; i++) {
		aiVector3D vector3D = inputMesh->mVertices[i];

		mesh->vertices.push_back(vector3D.x);
		mesh->vertices.push_back(vector3D.y);
		mesh->vertices.push_back(vector3D.z);
	}

	// texCoords
	for (unsigned int i = 0; i < numVertices; i++) {
		aiVector3D vector3D = inputMesh->mTextureCoords[0][i]; // can have 8 sets of texCoords...only need the first
		
		mesh->texCoords.push_back(vector3D.x);
		mesh->texCoords.push_back(vector3D.y);
	}

	// normals
	for (unsigned int i = 0; i < numVertices; i++) {
		aiVector3D vector3D = inputMesh->mNormals[i];

		mesh->normals.push_back(vector3D.x);
		mesh->normals.push_back(vector3D.y);
		mesh->normals.push_back(vector3D.z);
	}

	// indices
	for (unsigned int i = 0; i < inputMesh->mNumFaces; i++) {
		aiFace face = inputMesh->mFaces[i];

		// We did set the flag to triangulate the indices, but just in case...
		if (face.mNumIndices != 3) {
			continue;
		}

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			mesh->indices.push_back(face.mIndices[j]);
		}		
	}

	// VAO setup
	glGenVertexArrays(1, &mesh->VAO_ID);
	glBindVertexArray(mesh->VAO_ID);

	unsigned int VerticesVBO_ID;
	glGenBuffers(1, &VerticesVBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, VerticesVBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->vertices[0]) * mesh->vertices.size(), &mesh->vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int TexCoordsVBO_ID;
	glGenBuffers(1, &TexCoordsVBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, TexCoordsVBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->texCoords[0]) * mesh->texCoords.size(), &mesh->texCoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	unsigned int NormalsVBO_ID;
	glGenBuffers(1, &NormalsVBO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, NormalsVBO_ID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh->normals[0]) * mesh->normals.size(), &mesh->normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	unsigned int EBO_ID;
	glGenBuffers(1, &EBO_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->indices[0]) * mesh->indices.size(), &mesh->indices[0], GL_STATIC_DRAW);

	unsigned int matIndex = inputMesh->mMaterialIndex;
	aiMaterial *mat = scene->mMaterials[matIndex];

	unsigned int ambientTextureCount = mat->GetTextureCount(aiTextureType_AMBIENT);
	for (unsigned int i = 0; i < ambientTextureCount; i++) {
		
		aiString fileName;
		mat->GetTexture(aiTextureType_AMBIENT, i, &fileName);
		
		mesh->ambientTextureFileNames.push_back(fileName.C_Str());
	}

	unsigned int diffuseTextureCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
	for (unsigned int i = 0; i < diffuseTextureCount; i++) {

		aiString fileName;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &fileName);

		mesh->diffuseTextureFileNames.push_back(fileName.C_Str());
	}

	unsigned int specularTextureCount = mat->GetTextureCount(aiTextureType_SPECULAR);
	for (unsigned int i = 0; i < specularTextureCount; i++) {

		aiString fileName;
		mat->GetTexture(aiTextureType_SPECULAR, i, &fileName);

		mesh->specularTextureFileNames.push_back(fileName.C_Str());
	}

	if (ambientTextureCount > 0) {
		std::cout << "ambient: " + *mat->GetName().C_Str() << std::endl;
	}

	if (diffuseTextureCount > 0) {
		std::cout << "diffuse: " + *mat->GetName().C_Str() << std::endl;
	}

	if (specularTextureCount > 0) {
		std::cout << "specular: " + *mat->GetName().C_Str() << std::endl;
	}

	// TODO: tangents and bitangents, when you know what those are about
}

#define MESH
#endif