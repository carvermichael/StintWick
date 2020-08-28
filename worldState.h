#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"
#include "model.h"
#include "entities.h"

struct Map {
	bool initialized = false;
	unsigned int grid[GRID_MAP_SIZE_Y][GRID_MAP_SIZE_X];

	int openings;
};

struct WorldState {
	unsigned int seed;

    Map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
    bool storePlaced = false;
    bool someOtherThingPlaced = false;

    int currentMapX = PLAYER_WORLD_START_X;
    int currentMapY = PLAYER_WORLD_START_Y;

    AABB wallBounds;

	Camera camera;

	Light light;

	Player player;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet bullets[MAX_BULLETS];

    WorldState() {
        // @HARDCODE: this is set relative to wall cube sizes
        wallBounds.AX =  1.0f;
        wallBounds.AY = -1.0f;

        wallBounds.BX =  (GRID_MAP_SIZE_X - 1); 
        wallBounds.BY = -(GRID_MAP_SIZE_Y - 1); 
    }

    Map *currentMap() {
        return &allMaps[currentMapX][currentMapY];
    }
};


glm::vec3 gridCoordsToWorldOffset(glm::ivec3 gridCoords) {
    glm::vec3 worldOffset;

    worldOffset.x = (float) gridCoords.x;
    worldOffset.y = (float)-gridCoords.y;
    worldOffset.z = (float) gridCoords.z;

    return worldOffset;
}

glm::ivec3 worldOffsetToGridCoords(glm::vec3 worldOffset) {
    
    glm::ivec3 gridCoords;

    // TODO: verify this truncation results in the desired result
    gridCoords.x = (int)( worldOffset.x); 
    gridCoords.y = (int)(-worldOffset.y); 
    gridCoords.z = (int)( worldOffset.z); 
    
    //printf("World: (%.2f, %.2f) --> Grid: (%i, %i)\n", worldOffset.x, worldOffset.y, gridCoords.x, gridCoords.y);

    return gridCoords;
}

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

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
