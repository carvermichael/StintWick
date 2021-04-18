#pragma once

#include <fstream>
#include <sstream>
#include "constants.h"
#include "math.h"

/*

	Level format:

		15 26  // player starting position - x y
	
		12	   // # wall pieces
		12 13  // x, y coords of walls
		12 14
		2 3
		2 4
		2 5
		1 5
		0 5
		2 6
		5 8
		4 7
		3 6
		2 1

		TODO: add floors here
		10		// # floor pieces
		2 3		// x, y coords of floors
		2 4
		2 5
		1 5
		0 5
		2 6
		5 8
		4 7
		3 6
		2 1

		9	   // # enemies
		1 8  4 // enemies - type, gridX, gridY
		1 12 4
		1 16 4
		1 20 4
		1 24 4
		2 12 6
		2 16 6
		2 20 6
		2 24 6

*/

struct Level {

	void init(my_ivec2 playerPos);
	
	void addEnemy(int type, my_ivec2 gridCoords);
	void removeEnemy(int index);

	bool initialized;

	// v1 stuff
	int sizeX;
	int sizeY;

	// v2 stuff
	unsigned int numWalls;
	my_ivec2 wallLocations[MAX_WALLS];

	unsigned int numFloors;
	my_ivec2 floorLocations[MAX_FLOORS];

	int numEnemies;

	int playerStartX;
	int playerStartY;

	struct enemy {
		int enemyType;
		int gridX;
		int gridY;
	};

	enemy enemies[MAX_ENEMIES];
};

struct Levels {

	

};