#include "levels.h"

void Level::init(my_ivec2 playerPos) {

	this->playerStartX = playerPos.x;
	this->playerStartY = playerPos.y;

	this->initialized = true;
}

void Level::addEnemy(int type, my_ivec2 gridCoords) {

	if (numEnemies >= MAX_ENEMIES) {
		printf("ERROR: Cannot add enemy to level, too many enemies.");
		return;
	}

	enemy *e = &enemies[numEnemies];

	e->enemyType = type;
	e->gridX = gridCoords.x;
	e->gridY = gridCoords.y;

	numEnemies++;
}

void Level::removeEnemy(int index) {
	for (int i = index; i < numEnemies - 1; i++) {
		enemies[i].enemyType = enemies[i + 1].enemyType;
		enemies[i].gridX = enemies[i + 1].gridX;
		enemies[i].gridY = enemies[i + 1].gridY;
	}

	numEnemies--;
}
