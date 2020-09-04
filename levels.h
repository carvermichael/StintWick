#pragma once

#include <fstream>

// TODO: set four different entrances/exits &
//		 also 4 different player spawn points

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

struct Level {

	bool initialized;

	int sizeX;
	int sizeY;

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

#define MAX_LEVELS 50

Level levels[MAX_LEVELS];

void loadLevels() {

	const char* fileName = "levels.lev";

	std::ifstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);

	std::stringstream levelStream;
	levelStream << levelFile.rdbuf();
	levelFile.close();

	int levelCount = 0;

	while (!levelStream.eof() && levelCount <= MAX_LEVELS) {
		Level *currentLevel = &levels[levelCount];
		levelStream >> currentLevel->sizeX;
		levelStream >> currentLevel->sizeY;

		levelStream >> currentLevel->playerStartX;
		levelStream >> currentLevel->playerStartY;

		levelStream >> currentLevel->numEnemies;		

		for (int i = 0; i < currentLevel->numEnemies; i++) {
			levelStream >> currentLevel->enemies[i].enemyType;
			levelStream >> currentLevel->enemies[i].gridX;
			levelStream >> currentLevel->enemies[i].gridY;
		}

		currentLevel->initialized = true;
		levelCount++;
	}	
}
