#pragma once

#include "constants.h"
#include "console.h"
#include "math.h"
#include <vector>

struct Material {
	std::string name;

	my_vec3 ambient;
	my_vec3 diffuse;
	my_vec3 specular;
	float shininess;

	Material();
	Material(std::string name, my_vec3 ambient, my_vec3 diffuse, my_vec3 specular, float shininess);
};

struct Materials {
	
	union {
		Material mats[NUM_MATS];

		struct {
			// Order matters here!! The order of enemy strats syncs with material order
			// such that a single index can signify an enemy's appearance and strategy.
			//				-- carver (9-10-20)

			// enemy mats
			Material gold;
			Material ruby;
			
			// other mats
			Material light;
			Material emerald;
			Material chrome;
			Material silver;
			Material yellow;
			Material blackRubber;			
			Material grey;	
		};
	};

	Materials();
	~Materials();
};

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

	unsigned int *shaderProgramID;

	// just cube for now
	// later, we can pass in some struct with vertices/indices
	Mesh(unsigned int *newShaderProgramID, Material *newMaterial);

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor);

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat);

	void rescale(my_vec3 scale);

	void scale(my_vec3 scale);

	void reloadToVBOs();

	void setupVAO();
};

// www.youtube.com/watch?v=BTGo-JHuCGc
struct Model {
	std::string name;

	std::vector<Mesh> meshes;

	my_vec3 scaleFactor = my_vec3(1.0f);	

	Model();

	Model(std::string name);

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor);

	void draw(my_vec3 worldOffset, float bodyFactor, float outlineFactor, Material *mat);

	void draw(my_vec3 worldOffset);

	void drawOnlyOutline(my_vec3 worldOffset);

	// puts vertices back to original, then scales
	void rescale(my_vec3 scale);

	// scales from current vertices
	void scale(my_vec3 scale);
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
			Model wall;

			Model lightCube;

			Model guidingGrid;
		};
	};
};
