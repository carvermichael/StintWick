#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"
#include "model.h"
#include "entities.h"

struct WorldState {
	unsigned int seed;

	unsigned int gridSizeX;
	unsigned int gridSizeY;

    AABB wallBounds;

	Camera camera;

	Light light;

	Player player;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet bullets[MAX_BULLETS];

    WorldState() {
		gridSizeX = GRID_MAP_SIZE_X;
		gridSizeY = GRID_MAP_SIZE_Y;

        // @HARDCODE: this is set relative to wall cube sizes
        wallBounds.AX =  1.0f;
        wallBounds.AY = -1.0f;

        wallBounds.BX =  (float)(gridSizeX - 1);
        wallBounds.BY = -(float)(gridSizeY - 1); 
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
