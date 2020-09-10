#pragma once

#include <fstream>

// TODO: set four different entrances/exits &
//		 also 4 different player spawn points

// TODO: names for levels??

/*
	9-5-20 TODO - Level Editor:

	- when click, add enemies to Level struct - X
	- have button for saving - X - F5
		- which then persists to levels.lev - X
	- UI interaction for selecting levels - maybe later, doing console commands for now
		- on mouse click, first check for UI interactions
		- create textBox with current level number
*/

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

	void addEnemy(int type, my_ivec2 gridCoords) {

		if (numEnemies >= MAX_ENEMIES) {
			addTextToBox("ERROR: Cannot add enemy to level, too many enemies.", &eventTextBox);
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

#define MAX_LEVELS 50

Level levels[MAX_LEVELS];
unsigned int levelCount;

void addLevel(unsigned int gridSizeX, unsigned int gridSizeY) {

	// TODO: maybe have max gridSize???

	levels[levelCount].sizeX = gridSizeX;
	levels[levelCount].sizeY = gridSizeY;

	levels[levelCount].playerStartX = gridSizeX / 2;
	levels[levelCount].playerStartY = gridSizeY - 2;

	levels[levelCount].initialized = true;
	levelCount++;
}

void loadLevels() {

	const char* fileName = "levels.lev";

	std::ifstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);

	std::stringstream levelStream;
	levelStream << levelFile.rdbuf();
	levelFile.close();

	levelCount = 0;
	unsigned int numLevels;

	levelStream >> numLevels;

	while (!levelStream.eof() && levelCount <= MAX_LEVELS && levelCount < numLevels) {
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

void saveAllLevels() {

	// TODO: logging and error handling
	// TODO: more robust solution --> write to temp file with timestamp, then switch names

	addTextToBox("Saving...", &eventTextBox);

	std::stringstream stringStream;

	stringStream << std::to_string(levelCount) + "\n\n";
	
	for (int i = 0; i < MAX_LEVELS; i++) {
		Level *currentLevel = &levels[i];
		if (!currentLevel->initialized) break;

		stringStream << std::to_string(currentLevel->sizeX) + " " + std::to_string(currentLevel->sizeY) + "\n";
		stringStream << std::to_string(currentLevel->playerStartX) + " " + std::to_string(currentLevel->playerStartY) + "\n";
		stringStream << std::to_string(currentLevel->numEnemies) + "\n";

		for (int j = 0; j < currentLevel->numEnemies; j++) {
			stringStream << std::to_string(currentLevel->enemies[j].enemyType) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridX) + " ";
			stringStream << std::to_string(currentLevel->enemies[j].gridY) + "\n";
		}

		stringStream << "\n";
	}

	const char* fileName = "levels.lev";
	std::ofstream levelFile;
	levelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	levelFile.open(fileName);
	levelFile.clear();
	levelFile << stringStream.str();
	levelFile.close();

	addTextToBox("Saving complete.", &eventTextBox);
}
