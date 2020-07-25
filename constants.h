#if !defined(CONSTANTS)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define UP		1
#define DOWN	2
#define LEFT	4
#define RIGHT	8

// Room Types
#define NORMAL			0
#define STORE			1
#define SOMEOTHERTHING	2

#define WORLD_MAP_SIZE_X 5
#define WORLD_MAP_SIZE_Y 5

// maps must be even and square
#define GRID_MAP_SIZE_X	8
#define GRID_MAP_SIZE_Y	8

#define PLAYER_WORLD_START_X 2
#define PLAYER_WORLD_START_Y 2

#define PLAYER_GRID_START_X 4
#define PLAYER_GRID_START_Y 4

#define ACTION_STATE_SEEKING	0
#define ACTION_STATE_AVOIDANT	1

#define PLAYER_SPEED 1

struct map {
	bool initialized = false;
	unsigned int grid[GRID_MAP_SIZE_Y][GRID_MAP_SIZE_X];
};

struct character {
	int worldCoordX;
	int worldCoordY;
	int gridCoordX;
	int gridCoordY;

	int shaderProgramID;

	int directionFacing;
	int actionState;

	int hitPoints;
	int strength;
};

struct worldState {
	// This setup will result in a sparse world map. Not a big deal for now, but there is a risk for memory explosion if the size of the possible map expands. (carver - 7-20-20)
	map allMaps[WORLD_MAP_SIZE_X][WORLD_MAP_SIZE_Y];
	bool storePlaced = false;
	bool someOtherThingPlaced = false;

	character player;
	character theOther;
};


#define CONSTANTS 0
#endif // !CONSTANTS


