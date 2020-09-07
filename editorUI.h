#pragma once

#include "math.h"
#include "UIRect.h"
#include "textBox.h"

struct EditorUI {
	Font *font;

	unsigned int shaderProgramID;

	AABB bounds;
	float width;
	float height;

	float topBuffer;
	float bottomBuffer;

	UI_Rect background;

	// level selector
	UI_Rect leftArrow;
	UI_Rect rightArrow;
	UI_Rect levelIndicator;
	
	void setup(unsigned int shaderProgramID, Font *font) {
		this->font = font;

		width = currentScreenWidth / 4.0f;
		height = (float)currentScreenHeight;

		topBuffer = currentScreenHeight / 25.0f;
		bottomBuffer = currentScreenHeight / 25.0f;

		bounds.AX = (float)currentScreenWidth - width;
		bounds.AY = (float)currentScreenHeight;
		
		bounds.BX = bounds.AX + width;
		bounds.BY = bounds.AY - height;

		background.setup(shaderProgramID);
		background.color = my_vec4(0.75f, 0.1f, 0.1f, 0.3f);
		background.setBounds(bounds);

		leftArrow.setup(shaderProgramID);
		leftArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
		
		// TODO: just set position and width/height --> UIRect should work out bounds from there
		leftArrow.height = (height - topBuffer - bottomBuffer) / 20.0f;
		leftArrow.width = width / 4.0f;
		leftArrow.bounds.AX = bounds.AX;
		leftArrow.bounds.AY = currentScreenHeight - topBuffer;
		leftArrow.bounds.BX = leftArrow.bounds.AX + leftArrow.width;
		leftArrow.bounds.BY = leftArrow.bounds.AY - leftArrow.height;

		rightArrow.setup(shaderProgramID);
		rightArrow.color = my_vec4(0.25f, 0.05f, 0.05f, 0.6f);
		rightArrow.height = leftArrow.height;
		rightArrow.width = leftArrow.width;
		rightArrow.bounds.AX = bounds.BX - rightArrow.width;
		rightArrow.bounds.AY = currentScreenHeight - topBuffer;
		rightArrow.bounds.BX = rightArrow.bounds.AX + rightArrow.width;
		rightArrow.bounds.BY = rightArrow.bounds.AY - rightArrow.height;

		levelIndicator.setup(shaderProgramID);
		levelIndicator.color = my_vec4(0.25f, 0.25f, 0.05f, 0.6f);
		levelIndicator.height = leftArrow.height;
		levelIndicator.width = width - leftArrow.width - rightArrow.width;
		levelIndicator.bounds.AX = leftArrow.bounds.BX;
		levelIndicator.bounds.AY = currentScreenHeight - topBuffer;
		levelIndicator.bounds.BX = levelIndicator.bounds.AX + levelIndicator.width;
		levelIndicator.bounds.BY = levelIndicator.bounds.AY - levelIndicator.height;
	}

	void draw() {
		background.draw();
		leftArrow.draw();
		rightArrow.draw();

		levelIndicator.draw();
		std::string levelIndicatorString = "Level: " + std::to_string(getCurrentLevel());

		drawText(font, levelIndicatorString, levelIndicator.bounds.AX + 8.0f, levelIndicator.bounds.BY + 4.0f, 0.5f, my_vec3(1.0f, 1.0f, 1.0f));
	}

	boolean click(my_vec2 clickCoords) {
		if (!background.click(clickCoords)) return false;
		
		if (leftArrow.click(clickCoords)) {
			goBackOneLevel();
			return true;
		}

		if (rightArrow.click(clickCoords)) {
			goForwardOneLevel();
			return true;
		}		
		
		return true;
	}

	void refresh() {
		setup(shaderProgramID, font);
	}
};

