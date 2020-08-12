#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"
#include "model.h"

struct Map {
	bool initialized = false;
	unsigned int grid[GRID_MAP_SIZE_Y][GRID_MAP_SIZE_X];

	int openings;
};

struct Entity {
	Model *model;

	int directionFacing = DOWN;
	glm::vec3 worldOffset;

	int worldCoordX;
	int worldCoordY;
	
	glm::uvec3 gridCoords;

	int actionState;

	int hitPoints;
	int strength;

	void draw(Light light) {
		worldOffset.x =  0.5f * gridCoords.x;
		worldOffset.y = -0.5f * gridCoords.y;
		worldOffset.z =  0.5f * gridCoords.z;
		
		model->draw(worldOffset, directionFacing, light);
	}
};

struct WorldState {
	// This setup will result in a sparse world map. Not a big deal for now, but there is a risk for memory explosion if the size of the possible map expands. (carver - 7-20-20)
	Map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

	Camera camera;

	Light light;

	//Entity lightEntity;
	Entity player;
	Entity enemy;

	// TODO: maybe put a draw call here for the entire world state???
};

WorldState world;

#define WORLDSTATE
#endif