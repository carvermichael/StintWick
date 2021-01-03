#pragma once

#include "math.h"
#include "UIRect.h"
#include "textBox.h"

typedef void (*goBackOneEnemyType)(unsigned int*);
typedef void (*goForwardOneEnemyType)(unsigned int*);
typedef void (*toggleEntityType)(unsigned int*);

struct Selector {
	unsigned int shaderProgramID;
	Font *font;
	
	unsigned int *editorMode;
	unsigned int *enemySelection;

	my_vec2 location;
	float height;
	float width;

	UI_Rect leftArrow;
	UI_Rect rightArrow;
	UI_Rect stateIndicator;

	goBackOneEnemyType gboet;
	goForwardOneEnemyType gfoet;
	toggleEntityType tet;

	void setup(goBackOneEnemyType gboet, goForwardOneEnemyType gfoet, toggleEntityType tet,				
				unsigned int *editorMode, unsigned int *enemySelection, unsigned int inShaderProgramID,
				Font *inFont, my_vec2 *inTopLeft, float *inHeight, float *inWidth);
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

struct EditorUI {
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

	goBackOneEnemyType gboet;
	goForwardOneEnemyType gfoet;
	toggleEntityType tet;

	unsigned int *editorMode;
	unsigned int *enemySelection;

	void setup(goBackOneEnemyType inGboet, goForwardOneEnemyType inGfoet, toggleEntityType inTet, 
				unsigned int *editorMode, unsigned int *enemySelection, unsigned int inShaderProgramID, Font *inFont, float screenWidth, float screenHeight);
	void draw();

	void leftClick(my_vec2 clickCoords, my_ivec3 gridCoords);
	void rightClick(my_ivec3 gridCoords);


	bool click(my_vec2 clickCoords);
	void refresh(float screenWidth, float screenHeight);
};

