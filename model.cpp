#include "model.h"
#include "global_manip.h"

// MATERIAL
Material::Material() {};

Material::Material(std::string name, my_vec3 ambient, my_vec3 diffuse, my_vec3 specular, float shininess) {
	this->name = name;

	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
	this->shininess = shininess;
}

// MATERIALS
Materials::Materials() {
	// pulled from http://devernay.free.fr/cours/opengl/materials.html
	emerald = Material("emerald", my_vec3(0.0215f, 0.1745f, 0.0215f),
		my_vec3(0.07568f, 0.61424f, 0.07568f),
		my_vec3(0.633f, 0.727811f, 0.633f),
		0.6f);

	chrome = Material("chrome", my_vec3(0.25f, 0.25f, 0.25f),
		my_vec3(0.4f, 0.4f, 0.4f),
		my_vec3(0.774597f, 0.774597f, 0.774597f),
		0.6f);

	silver = Material("silver", my_vec3(0.19225f, 0.19225f, 0.19225f),
		my_vec3(0.50754f, 0.50754f, 0.50754f),
		my_vec3(0.508273f, 0.508273f, 0.508273f),
		0.4f);

	gold = Material("gold", my_vec3(0.24725f, 0.1995f, 0.0745f),
		my_vec3(0.75164f, 0.60648f, 0.22648f),
		my_vec3(0.628281f, 0.555802f, 0.366065f),
		0.4f);

	blackRubber = Material("blackRubber", my_vec3(0.02f, 0.02f, 0.02f),
		my_vec3(0.01f, 0.01f, 0.01f),
		my_vec3(0.4f, 0.4f, 0.4f),
		0.78125f); // upped this from 0.078125

	ruby = Material("ruby", my_vec3(0.1745f, 0.01175f, 0.01175f),
		my_vec3(0.61424f, 0.04136f, 0.04136f),
		my_vec3(0.727811f, 0.626959f, 0.626959f),
		0.6f);

	light = Material("light", my_vec3(1.0f),
		my_vec3(1.0f),
		my_vec3(1.0f),
		1.0f);

	grey = Material("grey", my_vec3(0.23f),
		my_vec3(0.23f),
		my_vec3(0.23f),
		1.0f);

	yellow = Material("yellow", my_vec3(0.05f, 0.05f, 0.0f),
		my_vec3(0.5f, 0.5f, 0.4f),
		my_vec3(0.7f, 0.7f, 0.04f),
		0.078125f);
}

Materials::~Materials() {};

// MESH

// just cube for now
// later, we can pass in some struct with vertices/indices
Mesh::Mesh(unsigned int *newShaderProgramID, Material *newMaterial) {
	// SHAPE DATA
	float cubeVertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,

		// bottom
		// 5, 6, 7, 8
		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		1.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// left
		// 1, 4, 5, 8
		0.0f, 1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// right
		// 2, 3, 6, 7
		1.0f, 1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

		// back		
		// 1, 2, 5, 6
		0.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
	};
	unsigned int cubeIndices[] = {
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
	unsigned int cubeOutlineIndices[] = {
		0, 4, 0, 1, 5, 1,
		2, 6, 2, 3, 7, 3,
		0, 4, 5, 6, 7, 4
	};
	
	shaderProgramID = newShaderProgramID;
	material = newMaterial;

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		vertices.push_back(cubeVertices[i]);
	}

	for (int i = 0; i < sizeof(cubeIndices) / sizeof(unsigned int); i++) {
		indices.push_back(cubeIndices[i]);
	}

	for (int i = 0; i < sizeof(cubeOutlineIndices) / sizeof(unsigned int); i++) {
		outlineIndices.push_back(cubeOutlineIndices[i]);
	}

	setupVAO();
};

void Mesh::draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor) {
	draw(worldOffset, bodyFactor, outlineFactor, this->material);
}

void Mesh::draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat) {
	//printf("Drawing with ID: %d\n", *shaderProgramID);

	glUseProgram(*shaderProgramID);
	glBindVertexArray(VAO_ID);
	my_mat4 current_model = translate(my_mat4(1.0f), worldOffset);
	setUniformMat4(*shaderProgramID, "model", current_model);

	if (bodyFactor > 0.0f) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
		glDepthFunc(GL_LESS);

		setUniform3f(*shaderProgramID, "materialAmbient", mat->ambient * bodyFactor);
		setUniform3f(*shaderProgramID, "materialDiffuse", mat->diffuse * bodyFactor);
		setUniform3f(*shaderProgramID, "materialSpecular", mat->specular * bodyFactor);
		setUniform1f(*shaderProgramID, "materialShininess", mat->shininess);

		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	}

	// ughhhh, this is so hacky...
	//outlineFactor = 1.0f;
	if (outlineFactor > 0.0f) { // could I just let this fly, do the draw with the zero mult?
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
		glDepthFunc(GL_LEQUAL);
		//glDepthFunc(GL_LESS);
		setUniform3f(*shaderProgramID, "materialAmbient", my_vec3(1.0f) * outlineFactor);
		setUniform3f(*shaderProgramID, "materialDiffuse", my_vec3(1.0f) * outlineFactor);
		setUniform3f(*shaderProgramID, "materialSpecular", my_vec3(1.0f) * outlineFactor);
		setUniform1f(*shaderProgramID, "materialShininess", 100.0f);

		// hypotenuses
		glDrawElements(GL_LINES_ADJACENCY, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

		// outline of squares
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_EBO_ID);
		glDrawElements(GL_LINE_LOOP, (GLsizei)outlineIndices.size(), GL_UNSIGNED_INT, 0);
	}
}

void Mesh::rescale(my_vec3 scale) {
	// SHAPE DATA
	float cubeVertices[] = {
		// top
		// 1, 2, 3, 4
		0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,

		// bottom
		// 5, 6, 7, 8
		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		1.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,

		// left
		// 1, 4, 5, 8
		0.0f, 1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,

		// right
		// 2, 3, 6, 7
		1.0f, 1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		// front
		// 4, 3, 8, 7
		0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,

		// back		
		// 1, 2, 5, 6
		0.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
	};
	
	vertices.clear();

	for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
		vertices.push_back(cubeVertices[i]);
	}

	this->scale(scale);
}

void Mesh::scale(my_vec3 scale) {
	// NOTE: stride of 6 accounts for normals
	for (int i = 0; i < vertices.size(); i += 6) {
		vertices[i] *= scale.x;
		vertices[i + 1] *= scale.y;
		vertices[i + 2] *= scale.z;
	}

	// Probably a better way to make this transformation in place with openGL.
	reloadToVBOs();
}

void Mesh::reloadToVBOs() {
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO_ID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->vertices[0]) * this->vertices.size(), &this->vertices[0]);
}

void Mesh::setupVAO() {
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

	glGenBuffers(1, &outline_EBO_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_EBO_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->outlineIndices[0]) * this->outlineIndices.size(), &this->outlineIndices[0], GL_STATIC_DRAW);
}


// MODEL
Model::Model() {
	scaleFactor = my_vec3(1.0f);
}

Model::Model(std::string name) {
	this->name = name;
	scaleFactor = my_vec3(1.0f);
}

void Model::draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].draw(worldOffset, bodyFactor, outlineFactor);
	}
}

void Model::draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].draw(worldOffset, bodyFactor, outlineFactor, mat);
	}
}

void Model::draw(my_vec3 worldOffset) {
	draw(worldOffset, 1.0f, 0.0f);
}

void Model::drawOnlyOutline(my_vec3 worldOffset) {
	draw(worldOffset, 1.0f, 1.0f);
}

// puts vertices back to original, then scales
void Model::rescale(my_vec3 scale) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].rescale(scale);
	}
}

// scales from current vertices
void Model::scale(my_vec3 scale) {
	if (scaleFactor.x != 0.0f) scaleFactor.x *= scale.x;
	else					  scaleFactor.x = scale.x;

	if (scaleFactor.y != 0.0f) scaleFactor.y *= scale.y;
	else					   scaleFactor.y = scale.y;

	if (scaleFactor.z != 0.0f) scaleFactor.z *= scale.z;
	else					   scaleFactor.z = scale.z;

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].scale(scale);
	}
}