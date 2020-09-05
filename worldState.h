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
	int numEnemies = 0;
    Enemy enemies[MAX_ENEMIES];
    
    Bullet playerBullets[MAX_BULLETS];
	Bullet enemyBullets[MAX_BULLETS];

	ParticleEmitter particleEmitters[MAX_PARTICLE_EMITTERS];

    void init(unsigned int newGridSizeX, unsigned int newGridSizeY) {
		gridSizeX = newGridSizeX;
		gridSizeY = newGridSizeY;

        // @HARDCODE: this is set relative to wall cube sizes
        wallBounds.AX =  1.0f;
        wallBounds.AY = -1.0f;

        wallBounds.BX =  (float)(gridSizeX - 1);
        wallBounds.BY = -(float)(gridSizeY - 1);

		light.pos = glm::vec3(-2.0f, -5.0f, 4.0f);

		numEnemies = 0;
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

    gridCoords.x = (int)( worldOffset.x); 
    gridCoords.y = (int)(-worldOffset.y); 
    gridCoords.z = (int)( worldOffset.z); 
    
    return gridCoords;
}

#define MAX_ENTITIES 100
#define NUM_TURN_ENTITIES 2

/*

models
worldState
inputReplay
sounds

shaderProgramIDs

// probably don't need in this grouping
fonts
console

*/

#define WORLDSTATE
#endif
