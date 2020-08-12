#if !defined(WORLDGENERATION)
#include "constants.h"
#include "worldState.h"
#include <stdlib.h>
#include <glfw3.h>

void createSingleGrid(WorldState *world, int worldMapX, int worldMapY, int openings, int roomType);
void createAdjacentMaps(WorldState *world, int attachedWorldMapX, int attachedWorldMapY, int directionToGetHere);

void generateWorldMap(WorldState *world) {
	createSingleGrid(world, PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, UP | DOWN | LEFT | RIGHT, false);
	world->allMaps[PLAYER_WORLD_START_X][PLAYER_WORLD_START_Y].initialized = true;

	createAdjacentMaps(world, PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, UP);
	createAdjacentMaps(world, PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, DOWN);
	createAdjacentMaps(world, PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, LEFT);
	createAdjacentMaps(world, PLAYER_WORLD_START_X, PLAYER_WORLD_START_Y, RIGHT);
}

void createSingleGrid(WorldState *world, int worldMapX, int worldMapY, int openings, int roomType) {

	int newGrid[GRID_MAP_SIZE_X][GRID_MAP_SIZE_Y] = {};

	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			if (row == 0 || row == GRID_MAP_SIZE_X - 1 ||
				column == 0 || column == GRID_MAP_SIZE_Y - 1) {
				newGrid[row][column] = 1;
			}
		}
	}

	// NOTE: Relies on even rows/columns to keep exits centered.
	if (openings & LEFT) { // up
		newGrid[GRID_MAP_SIZE_X / 2 - 2][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 - 1][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2][0] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 + 1][0] = 0;
	}

	if (openings & RIGHT) { // down
		newGrid[GRID_MAP_SIZE_X / 2 - 2][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 - 1][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2][GRID_MAP_SIZE_Y - 1] = 0;
		newGrid[GRID_MAP_SIZE_X / 2 + 1][GRID_MAP_SIZE_Y - 1] = 0;
	}

	if (openings & UP) { // left
		newGrid[0][GRID_MAP_SIZE_Y / 2 - 2] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2 - 1] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2] = 0;
		newGrid[0][GRID_MAP_SIZE_Y / 2 + 1] = 0;
	}

	if (openings & DOWN) { // right
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 - 2] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 - 1] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2] = 0;
		newGrid[GRID_MAP_SIZE_X - 1][GRID_MAP_SIZE_Y / 2 + 1] = 0;
	}

	if (roomType == STORE) {
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 2] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2] = 1;
		newGrid[2][GRID_MAP_SIZE_Y / 2 + 1] = 1;
	}

	if (roomType == SOMEOTHERTHING) {
		newGrid[2][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[3][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[4][GRID_MAP_SIZE_Y / 2 - 1] = 1;
		newGrid[5][GRID_MAP_SIZE_Y / 2 + 0] = 1;
	}

	for (int row = 0; row < GRID_MAP_SIZE_X; row++) {
		for (int column = 0; column < GRID_MAP_SIZE_Y; column++) {
			world->allMaps[worldMapX][worldMapY].grid[row][column] = newGrid[row][column];
		}
	}

	// Setting random walls within playable subsection of grid
	//for (int row = 1; row < GRID_MAP_SIZE_X - 1; row++) {
	//	for (int column = 1; column < GRID_MAP_SIZE_Y - 1; column++) {
	//		if (rand() % 10 == 3) {
	//			world->allMaps[worldMapX][worldMapY].grid[row][column] = 1;
	//		}
	//	}
	//}

	world->allMaps[worldMapX][worldMapY].openings = openings;
}

void createAdjacentMaps(WorldState *world, int attachedWorldMapX, int attachedWorldMapY, int directionToGetHere) {
	// TODO: fix this directionToGetHere garbage, way too confusing -- just do cardinal directions

	int newGridX;
	int newGridY;
	int openings;

	switch (directionToGetHere) {
	case (UP):
		newGridX = attachedWorldMapX;
		newGridY = attachedWorldMapY + 1;
		openings = DOWN;
		break;
	case (DOWN):
		newGridX = attachedWorldMapX;
		newGridY = attachedWorldMapY - 1;
		openings = UP;
		break;
	case (LEFT):
		newGridX = attachedWorldMapX - 1;
		newGridY = attachedWorldMapY;
		openings = RIGHT;
		break;
	case (RIGHT):
		newGridX = attachedWorldMapX + 1;
		newGridY = attachedWorldMapY;
		openings = LEFT;
		break;
	}

	world->allMaps[newGridX][newGridY].initialized = true;
	int roomType = NORMAL;

	// generate down
	if (directionToGetHere != UP) {
		if (rand() % 4 == 2 && newGridY > 0 && !world->allMaps[newGridX][newGridY - 1].initialized) {
			openings |= DOWN;
			createAdjacentMaps(world, newGridX, newGridY, DOWN);
		}
	}

	// generate up
	if (directionToGetHere != DOWN) {
		if (rand() % 10 == 3 && !world->storePlaced) {
			roomType = STORE;
			world->storePlaced = true;
		}
		else if (rand() % 4 == 2 && newGridY <= WORLD_MAP_SIZE_Y && !world->allMaps[newGridX][newGridY + 1].initialized) {
			openings |= UP;
			createAdjacentMaps(world, newGridX, newGridY, UP);
		}
	}

	// generate right
	if (directionToGetHere != LEFT) {
		if (rand() % 4 == 2 && newGridX <= WORLD_MAP_SIZE_X && !world->allMaps[newGridX + 1][newGridY].initialized) {
			openings |= RIGHT;
			createAdjacentMaps(world, newGridX, newGridY, RIGHT);
		}
	}

	// generate left
	if (directionToGetHere != RIGHT) {
		if (rand() % 10 == 3 && !world->someOtherThingPlaced) {
			roomType = SOMEOTHERTHING;
			world->someOtherThingPlaced = true;
		}
		if (rand() % 4 == 2 && newGridX > 0 && !world->allMaps[newGridX - 1][newGridY].initialized) {
			openings |= LEFT;
			createAdjacentMaps(world, newGridX, newGridY, LEFT);
		}
	}

	createSingleGrid(world, newGridX, newGridY, openings, roomType);
}

void regenerateMap() {
	world = {};
	world.someOtherThingPlaced = false;
	world.storePlaced = false;
	generateWorldMap(&world);
}


#define WORLDGENERATION
#endif
