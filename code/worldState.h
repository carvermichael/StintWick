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
	glm::vec3 destinationWorldOffset;

	int worldCoordX;
	int worldCoordY;
	
	glm::ivec3 gridCoords;
	glm::ivec3 destinationGridCoords;

	int actionState;

	int hitPoints;
	int strength;

	float speed = 15.0f;

	bool update(float deltaTime) {

		destinationWorldOffset.x =  0.5f * gridCoords.x;
		destinationWorldOffset.y = -0.5f * gridCoords.y;
		destinationWorldOffset.z =  0.5f * gridCoords.z;

		if (gridCoords == destinationGridCoords) {
			worldOffset = destinationWorldOffset;
		}
		else {
			float distX = destinationWorldOffset.x - worldOffset.x;
			float distY = destinationWorldOffset.y - worldOffset.y;
			float distZ = destinationWorldOffset.z - worldOffset.z;

			if (glm::abs(distX) < 0.005f) worldOffset.x = destinationWorldOffset.x;
			if (glm::abs(distY) < 0.005f) worldOffset.y = destinationWorldOffset.y;
			if (glm::abs(distZ) < 0.005f) worldOffset.z = destinationWorldOffset.z;

			float distToMoveX = distX * deltaTime * speed;
			float distToMoveY = distY * deltaTime * speed;
			float distToMoveZ = distZ * deltaTime * speed;

			worldOffset.x += distToMoveX;
			worldOffset.y += distToMoveY;
			worldOffset.z += distToMoveZ;
		}

		if (worldOffset == destinationWorldOffset) return true;

		return false;
	}

	void draw(Light light) {
		model->draw(worldOffset, light);
	}
};

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

struct WorldState {
	unsigned int seed;

	// This setup will result in a sparse world map. Not a big deal for now, but there is a risk for memory explosion if the size of the possible map expands. (carver - 7-20-20)
	Map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

	Camera camera;

	Light light;

	//Entity lightEntity;
	Entity player;
	Entity enemy;

	Entity entities[MAX_ENTITIES];

	Entity *turnOrder[NUM_TURN_ENTITIES];
	unsigned int currentTurnEntity = 0;
	bool turnInProgress = false;

	WorldState() {
		turnOrder[0] = &player;
		turnOrder[1] = &enemy;
	}

	// TODO: maybe put a draw call here for the entire world state???
};


/*

models
worldState
inputReplay
sounds(when available)

shaderProgramIDs

// probably don't need in this grouping
fonts
console

*/

#define WORLDSTATE
#endif
