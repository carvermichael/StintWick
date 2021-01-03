#include "editorUI.h"
#include <string>
#include "global_manip.h"

// Selector
void Selector::setup(goBackOneEnemyType inGboet, goForwardOneEnemyType inGfoet, toggleEntityType inTet,
		unsigned int *inEditorMode, unsigned int *inEnemySelection,
		unsigned int inShaderProgramID, Font *inFont, my_vec2 *inTopLeft, float *inHeight, float *inWidth) {

	this->gboet = inGboet;
	this->gfoet = inGfoet;
	this->tet = inTet;
	
	this->editorMode = inEditorMode;
	this->enemySelection = inEnemySelection;

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

	std::string enemyIndicatorString = "Level: " + std::to_string(getCurrentLevel());
	drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// ENEMY SELECTOR
bool EnemySelector::click(my_vec2 clickCoords) {
	if (leftArrow.click(clickCoords)) {
		gboet(enemySelection);
		return true;
	}

	if (rightArrow.click(clickCoords)) {
		gfoet(enemySelection);
		return true;
	}

	return false;
}

void EnemySelector::draw() {
	leftArrow.draw();
	rightArrow.draw();
	stateIndicator.draw();

	std::string enemyIndicatorString = "E: " + std::to_string(*enemySelection);
	drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// Entity Type Selector
bool EntityTypeSelector::click(my_vec2 clickCoords) {
	if (stateIndicator.click(clickCoords)) {
		tet(editorMode);
		return true;
	}

	return false;
}

void EntityTypeSelector::draw() {
	stateIndicator.draw();

	std::string wallBoolText = "Mode: ";
	if (*editorMode == EDITOR_MODE_ENEMY) {
		wallBoolText += "Enemy";
	}
	else if (*editorMode == EDITOR_MODE_WALL) {
		wallBoolText += "Wall";
	}
	drawText(font, wallBoolText, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
}

// EDITOR UI
void EditorUI::setup(goBackOneEnemyType inGboet, goForwardOneEnemyType inGfoet, toggleEntityType inTet, 
						unsigned int *inEditorMode, unsigned int *enemySelection, unsigned int inShaderProgramID, 
						Font *inFont, float screenWidth, float screenHeight) {
	this->font = inFont;

	this->gboet = inGboet;
	this->gfoet = inGfoet;
	this->tet = inTet;
	
	this->editorMode = inEditorMode;
	this->enemySelection = enemySelection;

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

	levelSelector.setup(inGboet, inGfoet, inTet, inEditorMode, enemySelection, inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);
	enemySelector.setup(inGboet, inGfoet, inTet, inEditorMode, enemySelection, inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);
	wallSelector.setup(inGboet, inGfoet, inTet, inEditorMode, enemySelection, inShaderProgramID, inFont, &currentTopLeft, &remainingHeight, &remainingWidth);

	deleteButton.setup(inShaderProgramID);
	deleteButton.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
	deleteButton.setBounds(currentTopLeft, this->height / 20.0f, this->width);
}

void EditorUI::draw() {
	background.draw();
	levelSelector.draw();
	enemySelector.draw();
	wallSelector.draw();
	deleteButton.draw();
}

bool EditorUI::click(my_vec2 clickCoords) {
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

void EditorUI::refresh(float screenWidth, float screenHeight) {
	setup(gboet, gfoet, tet, editorMode, enemySelection, shaderProgramID, font, screenWidth, screenHeight);
}
