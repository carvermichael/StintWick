#pragma once

#include "worldState.h"
#include "editorUI.h"

struct Editor {
	unsigned int mode = EDITOR_MODE_ENEMY;
	unsigned int currentEnemyTypeSelection = 0;
	
	EditorUI editorUI;

	WorldState *world;
	Level *levels;
	Textbox *eventTextBox;
	EnemyStrats *enemyStrats;
	unsigned int *currentLevelIndex;
	unsigned int *levelCount;

	void init(unsigned int UIShaderProgramID, Font *font, float screenWidth, float screenHeight,
		WorldState *inWorld, Level inLevels[MAX_LEVELS], Textbox *inEventTextBox, EnemyStrats *inEnemyStrats,
		unsigned int *inCurrentLevelIndex, unsigned int *levelCount);

	void refresh(float screenWidth, float screenHeight);

	void draw();

	void leftClick(my_vec2 clickCoords, my_ivec3 gridCoords);

	void rightClick(my_ivec3 gridCoords);

	unsigned int getEnemySelection();

	void addEnemyToLevel(int type, my_ivec2 gridCoords);

	void addWallToCurrentLevel(my_ivec2 location);
};
