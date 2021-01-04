#pragma once

#include "math.h"
#include "UIRect.h"
#include "textBox.h"

struct Selector {
	unsigned int shaderProgramID;
	Font *font;
	
	my_vec2 location;
	float height;
	float width;

	UI_Rect leftArrow;
	UI_Rect rightArrow;
	UI_Rect stateIndicator;

	void setup(unsigned int inShaderProgramID, Font *inFont, my_vec2 *inTopLeft, float *inHeight, float *inWidth);
	void draw();
	bool click(my_vec2 clickCoords);
};

// TODO: see if these click and draw stubs here are needed (they're on the super struct as well)
struct LevelSelector : Selector {
	bool click(my_vec2 clickCoords);
	void draw();
};

struct EnemySelector : Selector {
	bool click(my_vec2 clickCoords);
	void draw();
};

struct EntityTypeSelector : Selector {
	bool click(my_vec2 clickCoords);
	void draw();
};

struct Editor {
	Font *font;

	unsigned int shaderProgramID;
	float currentScreenWidth;
	float currentScreenHeight;

	AABB bounds;
	float width;
	float height;

	UI_Rect background;

	LevelSelector levelSelector;
	EnemySelector enemySelector;
	EntityTypeSelector wallSelector;
	UI_Rect deleteButton;

	void setup(unsigned int inShaderProgramID, Font *inFont, float screenWidth, float screenHeight);
	void draw();

	void leftClick(my_vec2 clickCoords, my_ivec3 gridCoords);
	void rightClick(my_ivec3 gridCoords);
	
	bool click(my_vec2 clickCoords);
	void refresh(float screenWidth, float screenHeight);
};

