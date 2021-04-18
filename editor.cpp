#include "editor.h"
#include <string>
#include "global_manip.h"
#include "randomUtils.h"

// Selector
void Selector::setup(unsigned int inShaderProgramID, Font *inFont, my_vec2 *inTopLeft, float *inHeight, float *inWidth) {

	this->shaderProgramID = inShaderProgramID;
	this->font = inFont;

	location.x = inTopLeft->x;
	location.y = inTopLeft->y;

	this->width = *inWidth;
	this->height = *inHeight / 20.0f;

	float arrowWidth = *inWidth * 0.25f;
	float indicatorWidth = *inWidth * 0.50f;

	float currentX = inTopLeft->x;

	leftArrow.setup(shaderProgramID);
	leftArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
	leftArrow.setBounds(my_vec2(currentX, location.y), this->height, arrowWidth);
	currentX += arrowWidth;

	stateIndicator.setup(shaderProgramID);
	stateIndicator.color = my_vec4(0.25f, 0.25f, 0.05f, 0.6f);
	stateIndicator.setBounds(my_vec2(currentX, location.y), this->height, indicatorWidth);
	currentX += indicatorWidth;

	rightArrow.setup(shaderProgramID);
	rightArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
	rightArrow.setBounds(my_vec2(currentX, location.y), this->height, arrowWidth);
	currentX += indicatorWidth;

	inTopLeft->y -= this->height;
	*inHeight -= this->height;
}

// LEVEL SELECTOR
bool LevelSelector::click(my_vec2 clickCoords) {
	if (leftArrow.click(clickCoords)) {
		goBackOneLevel();
				
		return true;
	}

	if (rightArrow.click(clickCoords)) {
		goForwardOneLevel();
				
		return true;
	}

	return false;
}

void LevelSelector::draw() {
	leftArrow.draw();
	rightArrow.draw();
	stateIndicator.draw();

	std::string enemyIndicatorString = "Level: " + std::to_string(getCurrentLevelIndex());
	drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// ENEMY SELECTOR
bool EnemySelector::click(my_vec2 clickCoords) {
	if (leftArrow.click(clickCoords)) {
		goBackOneEnemyType();
		return true;
	}

	if (rightArrow.click(clickCoords)) {
		goForwardOneEnemyType();
		return true;
	}

	return false;
}

void EnemySelector::draw() {
	leftArrow.draw();
	rightArrow.draw();
	stateIndicator.draw();

	int enemyTypeSelection = getEnemyTypeSelection();
	std::string enemyIndicatorString = "E: " + std::to_string(enemyTypeSelection);
	drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// Entity Type Selector
bool EntityTypeSelector::click(my_vec2 clickCoords) {
	if (stateIndicator.click(clickCoords)) {
		toggleEditorMode();
		return true;
	}

	return false;
}

void EntityTypeSelector::draw() {
	stateIndicator.draw();

	std::string wallBoolText = "Mode: ";
	int editorMode = getEditorMode();
	if (editorMode == EDITOR_MODE_ENEMY) {
		wallBoolText += "Enemy";
	}
	else if (editorMode == EDITOR_MODE_WALL) {
		wallBoolText += "Wall";
	}
	else if (editorMode == EDITOR_MODE_FLOOR) {
		wallBoolText += "Floor";
	}
	else if (editorMode == EDITOR_MODE_FLOOR_FILL) {
		wallBoolText += "Fill Floor";
	}
	drawText(font, wallBoolText, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// EDITOR UI
void Editor::setup(unsigned int inShaderProgramID, Font *inFont, float screenWidth, float screenHeight) {
	this->font = inFont;

	this->currentScreenWidth = screenWidth;
	this->currentScreenHeight = screenHeight;

	width = currentScreenWidth / 4.0f;
	height = currentScreenHeight;

	bounds.AX = (float)currentScreenWidth - width;
	bounds.AY = (float)currentScreenHeight;

	bounds.BX = bounds.AX + width;
	bounds.BY = bounds.AY - height;

	background.setup(inShaderProgramID);
	background.color = my_vec4(0.75f, 0.1f, 0.1f, 0.3f);
	background.setBounds(bounds);

	my_vec2 currentTopLeft = my_vec2(bounds.AX, bounds.AY);
	float remainingHeight = this->height;
	float remainingWidth = this->width;

	levelSelector.setup(inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);
	enemySelector.setup(inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);
	wallSelector.setup(inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);

	deleteButton.setup(inShaderProgramID);
	deleteButton.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
	deleteButton.setBounds(currentTopLeft, this->height / 20.0f, this->width);
}

void Editor::draw() {
	background.draw();
	levelSelector.draw();
	enemySelector.draw();
	wallSelector.draw();
	deleteButton.draw();
}

void Editor::leftClick(my_vec2 clickCoords, my_ivec3 gridCoords) {
	// check in editor box
	if (click(clickCoords)) return;

	// TODO: Raycast from click coords instead of camera center.
	// Slight hack to get a working way to place enemies before groking a raycast from click coords. (carver - 9-09-2020)
	
	int editorMode = getEditorMode();
	if (editorMode == EDITOR_MODE_ENEMY) {
		int enemyTypeSelection = getEnemyTypeSelection();
		addEnemyToWorld(enemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
		addEnemyToCurrentLevel(enemyTypeSelection, my_ivec2(gridCoords.x, gridCoords.y));
	}
	else if (editorMode == EDITOR_MODE_WALL) {
		addWallToWorld(my_ivec2(gridCoords.x, gridCoords.y));
		addWallToCurrentLevel(my_ivec2(gridCoords.x, gridCoords.y));
	}
	else if (editorMode == EDITOR_MODE_FLOOR) {
		addFloorToWorld(my_ivec2(gridCoords.x, gridCoords.y));
		addFloorToCurrentLevel(my_ivec2(gridCoords.x, gridCoords.y));
	}
	else if (editorMode == EDITOR_MODE_FLOOR_FILL) {
		fillFloor(my_ivec2(gridCoords.x, gridCoords.y), getCurrentLevel());
	}
}

void Editor::rightClick(my_ivec3 gridCoords) {
	my_vec3 worldOffset = gridCoordsToWorldOffset(gridCoords);

	removeEntityFromWorld(worldOffset);

	removeEntityFromCurrentLevel(my_ivec2(gridCoords.x, gridCoords.y));
}

bool Editor::click(my_vec2 clickCoords) {
	// check entire bounds
	if (!background.click(clickCoords)) return false;

	// then, go through elements, return after first one gets dibs
	if (levelSelector.click(clickCoords)) return true;
	if (enemySelector.click(clickCoords)) return true;
	if (wallSelector.click(clickCoords)) return true;
	if (deleteButton.click(clickCoords)) {
		deleteCurrentLevel();

		return true;
	}

	return true;
}

void Editor::refresh(float screenWidth, float screenHeight) {
	setup(shaderProgramID, font, screenWidth, screenHeight);
}
