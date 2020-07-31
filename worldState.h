#if !defined(WORLDSTATE)

#include "constants.h"
#include "camera.h"

struct Map {
	bool initialized = false;
	unsigned int grid[GRID_MAP_SIZE_Y][GRID_MAP_SIZE_X];
};

struct Character {
	int worldCoordX;
	int worldCoordY;
	int gridCoordX;
	int gridCoordY;

	Model *model;

	int directionFacing;
	int actionState;

	int hitPoints;
	int strength;
};

struct WorldState {
	// This setup will result in a sparse world map. Not a big deal for now, but there is a risk for memory explosion if the size of the possible map expands. (carver - 7-20-20)
	Map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

	Camera camera;

	Character player;
	Character theOther;
};

#define WORLDSTATE
#endif