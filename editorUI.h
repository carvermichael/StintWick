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

	void setup(unsigned int shaderProgramID, Font *font, my_vec2 *topLeft, float *height, float *width) {
		this->font = font;

		location.x = topLeft->x;
		location.y = topLeft->y;

		this->width = *width;
		this->height = *height / 20.0f;

		float arrowWidth		= *width * 0.25f;
		float indicatorWidth	= *width * 0.50f;

		float currentX = topLeft->x;
		
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

		topLeft->y	-= this->height;
		*height		-= this->height;
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

		std::string enemyIndicatorString = "E: " + std::to_string(getEnemySelection());
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

		my_vec2 currentTopLeft = my_vec2(bounds.AX, bounds.AY);
		float remainingHeight = this->height;
		float remainingWidth  = this->width;
		
		levelSelector.setup(shaderProgramID, font, &currentTopLeft, &remainingHeight, &remainingWidth);
		enemySelector.setup(shaderProgramID, font, &currentTopLeft, &remainingHeight, &remainingWidth);
	}

	void draw() {
		background.draw();
		levelSelector.draw();
		enemySelector.draw();
	}

	boolean click(my_vec2 clickCoords) {
		// check entire bounds
		if (!background.click(clickCoords)) return false;

		// then, go through elements, return after first one gets dibs
		if (levelSelector.click(clickCoords)) return true;
		if (enemySelector.click(clickCoords)) return true;
		
		return true;
	}

	void refresh() {
		setup(shaderProgramID, font);
	}
};

