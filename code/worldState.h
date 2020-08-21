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

	float speed = 0.2f;

	void draw(Light light) {
		model->draw(worldOffset, light);
	}
};

glm::vec3 gridCoordsToWorldOffset(glm::ivec3 gridCoords) {
    glm::vec3 worldOffset;

    worldOffset.x =  0.5f * gridCoords.x;
    worldOffset.y = -0.5f * gridCoords.y;
    worldOffset.z =  0.5f * gridCoords.z;

    return worldOffset;
}

glm::ivec3 worldOffsetToGridCoords(glm::vec3 worldOffset) {
    
    glm::ivec3 gridCoords;

    // TODO: verify this truncation results in the desired result
    gridCoords.x = (int)(worldOffset.x /  0.5f); 
    gridCoords.y = (int)(worldOffset.y / -0.5f); 
    gridCoords.z = (int)(worldOffset.z /  0.5f); 
    
    printf("World: (%.2f, %.2f) --> Grid: (%i, %i)\n", worldOffset.x, worldOffset.y, gridCoords.x, gridCoords.y);

    return gridCoords;
}

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

struct WorldState {
	unsigned int seed;

    Map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

    int currentMapX = PLAYER_WORLD_START_X;
    int currentMapY = PLAYER_WORLD_START_Y;

	Camera camera;

	Light light;

	Entity player;
	Entity enemy;

	Entity entities[MAX_ENTITIES];

    Map *currentMap() {
        return &allMaps[currentMapX][currentMapY];
    }
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
