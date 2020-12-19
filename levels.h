#pragma once

#include <fstream>
#include <sstream>
#include "constants.h"
#include "math.h"

// TODO: set four different entrances/exits &
//		 also 4 different player spawn points

// TODO: names for levels??

/*
	Example .lev format:

	30 30  // gridsize - x y
	15 26  // player starting position - x y
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

/*
New level format:

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

	bool initialized;

	// v1 stuff
	int sizeX;
	int sizeY;

	// v2 stuff
	unsigned int numWalls;
	my_ivec2 wallLocations[MAX_WALLS];

	int numEnemies;

	int playerStartX;
	int playerStartY;

	struct enemy {
		int enemyType;
		int gridX;
		int gridY;
	};

	enemy enemies[MAX_ENEMIES];

	void addEnemy(int type, my_ivec2 gridCoords, Textbox *eventTextbox) {

		if (numEnemies >= MAX_ENEMIES) {
			eventTextbox->addTextToBox("ERROR: Cannot add enemy to level, too many enemies.");
			return;
		}

		enemy *e = &enemies[numEnemies];

		e->enemyType = type;
		e->gridX = gridCoords.x;
		e->gridY = gridCoords.y;

		numEnemies++;
	}

	void removeEnemy(int index) {
		for (int i = index; i < numEnemies - 1; i++) {
			enemies[i].enemyType	= enemies[i + 1].enemyType;
			enemies[i].gridX		= enemies[i + 1].gridX;
			enemies[i].gridY		= enemies[i + 1].gridY;
		}

		numEnemies--;
	}	
};

unsigned int addLevel(Level levels[], unsigned int levelCount) {

	levels[levelCount].playerStartX = 100;
	levels[levelCount].playerStartY = 100;

	levels[levelCount].initialized = true;
	
	return levelCount + 1;
}

unsigned int loadLevelsV2(Level levels[]) {

	const char* fileName = "levels_v2.lev";

	std::ifstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);

	std::stringstream levelStream;
	levelStream << levelFile.rdbuf();
	levelFile.close();

	unsigned int levelCount = 0;
	unsigned int numLevels;

	levelStream >> numLevels;

	while (!levelStream.eof() && levelCount <= MAX_LEVELS && levelCount < numLevels) {
		Level *currentLevel = &levels[levelCount];
		
		levelStream >> currentLevel->playerStartX;
		levelStream >> currentLevel->playerStartY;

		levelStream >> currentLevel->numWalls;
		for (unsigned int i = 0; i < currentLevel->numWalls; i++) {
			levelStream >> currentLevel->wallLocations[i].x;
			levelStream >> currentLevel->wallLocations[i].y;
		}

		levelStream >> currentLevel->numEnemies;

		for (int i = 0; i < currentLevel->numEnemies; i++) {
			levelStream >> currentLevel->enemies[i].enemyType;
			levelStream >> currentLevel->enemies[i].gridX;
			levelStream >> currentLevel->enemies[i].gridY;
		}

		currentLevel->initialized = true;

		levelCount++;
	}

	return levelCount;
}

void saveAllLevelsV2(Level levels[], unsigned int levelCount, Textbox *eventTextbox) {

	// TODO: logging and error handling
	// TODO: more robust solution --> write to temp file with timestamp, then switch names

	eventTextbox->addTextToBox("Saving...");

	std::stringstream stringStream;

	stringStream << std::to_string(levelCount) + "\n\n";

	for (int i = 0; i < MAX_LEVELS; i++) {
		Level *currentLevel = &levels[i];
		if (!currentLevel->initialized) break;

		stringStream << std::to_string(currentLevel->playerStartX) + " " + std::to_string(currentLevel->playerStartY) + "\n";
		
		stringStream << std::to_string(currentLevel->numWalls) + "\n\n";
		
		for (unsigned int k = 0; k < currentLevel->numWalls; k++) {
			stringStream << std::to_string(currentLevel->wallLocations[k].x) + " " + std::to_string(currentLevel->wallLocations[k].y) + "\n";
		}
		stringStream << "\n";
		
		stringStream << std::to_string(currentLevel->numEnemies) + "\n";

		for (int j = 0; j < currentLevel->numEnemies; j++) {
			stringStream << std::to_string(currentLevel->enemies[j].enemyType) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridX) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridY) + "\n";
		}

		stringStream << "\n";
	}

	const char* fileName = "levels_v2.lev";
	std::ofstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);
	levelFile.clear();
	levelFile << stringStream.str();
	levelFile.close();

	eventTextbox->addTextToBox("Saving complete.");
}