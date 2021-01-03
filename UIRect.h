#pragma once

#include "math.h"

struct UI_Rect {

	bool initialized = false;

	unsigned int VAO_ID;
	unsigned int VBO_ID;
	unsigned int shaderProgramID;

	AABB bounds;
	float width;
	float height;
	
	my_vec4 color;

	void setup(unsigned int shaderProgramId);

	// TODO: remove, use bounds param instead
	// format (leftX, rightX, topY, bottomY)
	void setBounds(my_vec4 coords);

	void setBounds(AABB inBounds);

	void setBounds(my_vec2 topLeft, float inHeight, float inWidth);

	void draw();

	bool click(my_vec2 clickCoords);
};
