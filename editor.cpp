#include "editor.h"
#include "randomUtils.h"

void enemyTypeBack(unsigned int *enemyTypeSelection) {
	if (*enemyTypeSelection == 0) *enemyTypeSelection = NUM_ENEMY_TYPES - 1;
	else (*enemyTypeSelection)--;
}

void enemyTypeForward(unsigned int *enemyTypeSelection) {
	(*enemyTypeSelection)++;
	if ((*enemyTypeSelection) >= NUM_ENEMY_TYPES) *enemyTypeSelection = 0;
}

void backOneLevel(unsigned int *currentLevelIndex, unsigned int *levelCount) {
	if (*currentLevelIndex == 0) *currentLevelIndex = *levelCount - 1;
	else (*currentLevelIndex)--;
}

void forwardOneLevel(unsigned int *currentLevelIndex, unsigned int *levelCount) {
	(*currentLevelIndex)++;
	if ((*currentLevelIndex) >= *levelCount) *currentLevelIndex = 0;
}

void toggleTheEntityType(unsigned int *entityType) {
	if (*entityType == EDITOR_MODE_ENEMY) *entityType = EDITOR_MODE_WALL;
	else if (*entityType == EDITOR_MODE_WALL) *entityType = EDITOR_MODE_ENEMY;
}

void Editor::init(unsigned int UIShaderProgramID, Font *font, float screenWidth, float screenHeight,
	WorldState *inWorld, Level inLevels[MAX_LEVELS], Textbox *inEventTextBox, EnemyStrats *inEnemyStrats,
	unsigned int *inCurrentLevelIndex, unsigned int *inLevelCount) {
	this->world = inWorld;
	this->levels = inLevels;
	this->eventTextBox = inEventTextBox;
	this->enemyStrats = inEnemyStrats;
	this->currentLevelIndex = inCurrentLevelIndex;
	this->levelCount = inLevelCount;

	editorUI.setup(enemyTypeBack, enemyTypeForward, toggleTheEntityType, &mode, &currentEnemyTypeSelection, UIShaderProgramID, font, screenWidth, screenHeight);
}

void Editor::refresh(float screenWidth, float screenHeight) {
	editorUI.refresh(screenWidth, screenHeight);
}

void Editor::draw() {
	editorUI.draw();
}

void Editor::leftClick(my_vec2 clickCoords, my_ivec3 gridCoords) {
	// check in editor box
	if (editorUI.click(clickCoords)) return;

	// TODO: Raycast from click coords instead of camera center.
	// Slight hack to get a working way to place enemies before groking a raycast from click coords. (carver - 9-09-2020)


	if (mode == EDITOR_MODE_ENEMY) {
		world->addEnemyToWorld(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
		addEnemyToLevel(currentEnemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
	}
	else if (mode == EDITOR_MODE_WALL) {
		world->addWallToWorld(my_ivec2(gridCoords.x, gridCoords.y));
		addWallToCurrentLevel(my_ivec2(gridCoords.x, gridCoords.y));
	}
}

void Editor::rightClick(my_ivec3 gridCoords) {
	my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);

	world->removeEntityAtOffset(worldOffset);

	// remove target enemy from level
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (levels[*currentLevelIndex].enemies[i].gridX == gridCoords.x &&
			levels[*currentLevelIndex].enemies[i].gridY == gridCoords.y) {
			levels[*currentLevelIndex].removeEnemy(i);
		}
	}
}

unsigned int Editor::getEnemySelection() {
	return currentEnemyTypeSelection;
}

void Editor::addEnemyToLevel(int type, my_ivec2 gridCoords) {
	levels[*currentLevelIndex].addEnemy(type, gridCoords);
}

void Editor::addWallToCurrentLevel(my_ivec2 location) {
	unsigned int numWalls = levels[*currentLevelIndex].numWalls++;

	levels[*currentLevelIndex].wallLocations[numWalls] = location;
}
