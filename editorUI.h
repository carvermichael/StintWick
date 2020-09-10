#pragma once

#include "math.h"
#include "UIRect.h"
#include "textBox.h"

struct Selector {
	unsigned int shaderProgramID;
	Font *font;
	
	my_ivec2 location;

	// level selector
	// TODO:
	//	pull out of editorUI - X
	//	duplicate for selection of enemy type to place - X
	//  add bar above to indicate subject of selector
	//  new level button???
	UI_Rect leftArrow;
	UI_Rect rightArrow;
	UI_Rect stateIndicator; // TODO: name change

	float setup(unsigned int shaderProgramID, Font *font, AABB bounds) {
		this->font = font;
		
		leftArrow.setup(shaderProgramID);
		leftArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);

		float boundsWidth = bounds.BX - bounds.AX;
		float boundsHeight = bounds.AY - bounds.BY;

		// TODO: just set position and width/height --> UIRect should work out bounds from there
		leftArrow.height = (boundsHeight) / 20.0f;
		leftArrow.width = (boundsWidth) / 4.0f;
		leftArrow.bounds.AX = bounds.AX;
		leftArrow.bounds.AY = (float) currentScreenHeight;
		leftArrow.bounds.BX = leftArrow.bounds.AX + leftArrow.width;
		leftArrow.bounds.BY = leftArrow.bounds.AY - leftArrow.height;

		rightArrow.setup(shaderProgramID);
		rightArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
		rightArrow.height = leftArrow.height;
		rightArrow.width = leftArrow.width;
		rightArrow.bounds.AX = bounds.BX - rightArrow.width;
		rightArrow.bounds.AY = (float) currentScreenHeight;
		rightArrow.bounds.BX = rightArrow.bounds.AX + rightArrow.width;
		rightArrow.bounds.BY = rightArrow.bounds.AY - rightArrow.height;

		stateIndicator.setup(shaderProgramID);
		stateIndicator.color = my_vec4(0.25f, 0.25f, 0.05f, 0.6f);
		stateIndicator.height = leftArrow.height;
		stateIndicator.width = boundsWidth - leftArrow.width - rightArrow.width;
		stateIndicator.bounds.AX = leftArrow.bounds.BX;
		stateIndicator.bounds.AY = (float) currentScreenHeight;
		stateIndicator.bounds.BX = stateIndicator.bounds.AX + stateIndicator.width;
		stateIndicator.bounds.BY = stateIndicator.bounds.AY - stateIndicator.height;

		return stateIndicator.bounds.BY;
	}

	void draw() {
		leftArrow.draw();
		rightArrow.draw();
		stateIndicator.draw();
	
		std::string levelIndicatorString = "Level: " + std::to_string(getCurrentLevel());
		drawText(font, levelIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
	}

	bool click(my_vec2 clickCoords) {
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
};

struct LevelSelector : Selector {
	bool click(my_vec2 clickCoords) {
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

	void draw() {
		leftArrow.draw();
		rightArrow.draw();
		stateIndicator.draw();

		std::string enemyIndicatorString = "Level: " + std::to_string(getCurrentLevel());
		drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
	}
};

struct EnemySelector : Selector {
	bool click(my_vec2 clickCoords) {
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

	void draw() {
		leftArrow.draw();
		rightArrow.draw();
		stateIndicator.draw();

		std::string enemyIndicatorString = "EnemyType: " + std::to_string(getEnemySelection());
		drawText(font, enemyIndicatorString, stateIndicator.bounds.AX + 8.0f, stateIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
	}
};

struct EditorUI {
	Font *font;

	unsigned int shaderProgramID;

	AABB bounds;
	float width;
	float height;

	UI_Rect background;

	LevelSelector levelSelector;
	EnemySelector enemySelector;

	void setup(unsigned int shaderProgramID, Font *font) {
		this->font = font;

		width = currentScreenWidth / 4.0f;
		height = (float)currentScreenHeight;

		bounds.AX = (float)currentScreenWidth - width;
		bounds.AY = (float)currentScreenHeight;
		
		bounds.BX = bounds.AX + width;
		bounds.BY = bounds.AY - height;

		background.setup(shaderProgramID);
		background.color = my_vec4(0.75f, 0.1f, 0.1f, 0.3f);
		background.setBounds(bounds);

		// TODO: Rethink the bounds stuff here. This probably should just pass an initial (x,y) and (h,w).
		float newTop = levelSelector.setup(shaderProgramID, font, bounds);
		AABB remainingBounds = bounds;
		remainingBounds.AY = newTop;

		enemySelector.setup(shaderProgramID, font, remainingBounds);
	}

	void draw() {
		background.draw();
		levelSelector.draw();
		//enemySelector.draw();
	}

	boolean click(my_vec2 clickCoords) {
		// check entire bounds
		if (!background.click(clickCoords)) return false;

		// then, go through elements, return after first one gets dibs
		if (levelSelector.click(clickCoords)) return true;
		//if (enemySelector.click(clickCoords)) return true;
		
		return true;
	}

	void refresh() {
		setup(shaderProgramID, font);
	}
};

