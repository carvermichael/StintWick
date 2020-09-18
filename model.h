#if !defined(MODEL)

#include "shapeData.h"
#include "constants.h"
#include "console.h"
#include "math.h"

struct Material {
	std::string name;

	my_vec3 ambient;
	my_vec3 diffuse;
	my_vec3 specular;
	float shininess;

	Material() {};

	Material(std::string name, my_vec3 ambient, my_vec3 diffuse, my_vec3 specular, float shininess) {
		this->name = name;
		
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->shininess = shininess;
	}
};

struct Materials {
	
	union {
		Material mats[NUM_MATS];

		struct {
			// Order matters here!! The order of enemy strats syncs with material order
			// such that a single index can signify an enemy's appearance and strategy.
			//				-- carver (9-10-20)

			// enemy mats
			Material ruby;
			Material yellow;
			
			// other mats
			Material light;
			Material emerald;
			Material chrome;
			Material silver;
			Material gold;
			Material blackRubber;			
			Material grey;	
		};
	};

    Materials() {
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
											  0.078125f);

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

	~Materials() {};
};

Material* getMaterial(std::string name, Materials *materials) {
	for (int i = 0; i < NUM_MATS; i++) {
		if (materials->mats[i].name == name) {
			return &materials->mats[i];
		}
	}

	return NULL;
}

struct Mesh {

	std::vector<float> vertices;
	std::vector<float> texCoords;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> outlineIndices;

	float scaleFactor;

	Material *material;

	unsigned int VAO_ID;
	unsigned int verticesVBO_ID;
	unsigned int normalsVBO_ID;
	unsigned int EBO_ID;
	unsigned int outline_EBO_ID;

	unsigned int shaderProgramID;

	// just cube for now
	// later, we can pass in some struct with vertices/indices
	Mesh(unsigned int newShaderProgramID, Material *newMaterial) {
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

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor) {
		draw(worldOffset, bodyFactor, outlineFactor, this->material);
	}

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat) {
		glUseProgram(shaderProgramID);
		glBindVertexArray(VAO_ID);
		my_mat4 current_model = translate(my_mat4(1.0f), worldOffset);
		setUniformMat4(shaderProgramID, "model", current_model);

		if (bodyFactor > 0.0f) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);
			glDepthFunc(GL_LESS);

			setUniform3f(shaderProgramID, "materialAmbient", mat->ambient * bodyFactor);
			setUniform3f(shaderProgramID, "materialDiffuse", mat->diffuse * bodyFactor);
			setUniform3f(shaderProgramID, "materialSpecular", mat->specular * bodyFactor);
			setUniform1f(shaderProgramID, "materialShininess", mat->shininess);

			glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		}		

		if (outlineFactor > 0.0f) { // could I just let this fly, do the draw with the zero mult?
			setUniform3f(shaderProgramID, "materialAmbient", my_vec3(1.0f) * outlineFactor);
			setUniform3f(shaderProgramID, "materialDiffuse", my_vec3(1.0f) * outlineFactor);
			setUniform3f(shaderProgramID, "materialSpecular", my_vec3(1.0f) * outlineFactor);
			setUniform1f(shaderProgramID, "materialShininess", 100.0f);

			glDepthFunc(GL_LEQUAL);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_EBO_ID);
			glDrawElements(GL_LINE_LOOP, (GLsizei)outlineIndices.size(), GL_UNSIGNED_INT, 0);
		}
	}

	void rescale(my_vec3 scale) {
		vertices.clear();
		
		for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i++) {
			vertices.push_back(cubeVertices[i]);
		}

		this->scale(scale);
	}

	void scale(my_vec3 scale) {
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

		glGenBuffers(1, &outline_EBO_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_EBO_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->outlineIndices[0]) * this->outlineIndices.size(), &this->outlineIndices[0], GL_STATIC_DRAW);
	}
};

// www.youtube.com/watch?v=BTGo-JHuCGc
struct Model {
	std::string name;

	std::vector<Mesh> meshes;

	my_vec3 scaleFactor = my_vec3(1.0f);

	Model() {
		scaleFactor = my_vec3(1.0f);		
	}
	Model(std::string name) {
		this->name = name;
		scaleFactor = my_vec3(1.0f);
	}

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(worldOffset, bodyFactor, outlineFactor);
		}
	}

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(worldOffset, bodyFactor, outlineFactor, mat);
		}
	}

	void draw(my_vec3 worldOffset) {
		draw(worldOffset, 1.0f, 0.0f);
	}

	void drawOnlyOutline(my_vec3 worldOffset) {
		draw(worldOffset, 0.0f, 1.0f);
	}

	// puts vertices back to original, then scales
	void rescale(my_vec3 scale) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].rescale(scale);
		}
	}

	// scales from current vertices
	void scale(my_vec3 scale) {	
		if(scaleFactor.x != 0.0f) scaleFactor.x *= scale.x;
		else					  scaleFactor.x  = scale.x;

		if (scaleFactor.y != 0.0f) scaleFactor.y *= scale.y;
		else					   scaleFactor.y  = scale.y;

		if (scaleFactor.z != 0.0f) scaleFactor.z *= scale.z;
		else					   scaleFactor.z  = scale.z;
		
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].scale(scale);
		}
	}
};

struct Models {

	Models() {};
	~Models() {};

	union {
		Model mods[NUM_MODELS];

		struct {
			Model player;
			Model enemy;
			
            Model bullet;
			Model enemyBullet;
			Model bulletPart;

			Model floorModel;
			Model wallTopModel;
			Model wallLeftModel;

			Model lightCube;

			Model guidingGrid;
		};
	};
};

Model* getModel(std::string name, Models *models) {
	for (int i = 0; i < NUM_MODELS; i++) {
		if (models->mods[i].name == name) {
			return &models->mods[i];
		}
	}

	return NULL;
}

void setMaterial(std::string modelName, std::string matName, Materials *materials, Models *models, Console *console) {
	Material *mat = getMaterial(matName, materials);
	Model *model = getModel(modelName, models);
	bool fail = false;

	if (mat == NULL) {
		addTextToBox("Material not found: " + matName, &console->historyTextbox);
		fail = true;
	}
	if (model == NULL) {
		addTextToBox("Model not found: " + modelName, &console->historyTextbox);
		fail = true;
	}
	if (fail) return;

	for (int i = 0; i < model->meshes.size(); i++) {
		model->meshes[i].material = mat;
	}
}

#define MODEL
#endif