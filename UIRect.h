#pragma once

#include "math.h"

struct UI_Rect {

	// TODO: figure out why the linker threw errors when you tried to make some of these static
	bool initialized = false;

	unsigned int VAO_ID;
	unsigned int VBO_ID;
	unsigned int shaderProgramID;

	AABB bounds;
	float width;
	float height;
	
	my_vec4 color;

	void setup(unsigned int shaderProgramId) {
		if (initialized) return;

		glGenVertexArrays(1, &VAO_ID);
		glGenBuffers(1, &VBO_ID);
		glBindVertexArray(VAO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		color = my_vec4(0.2f, 0.2f, 0.2f, 0.7f);
		
		this->shaderProgramID = shaderProgramId;

		initialized = true;
	}

	// TODO: remove, use bounds param instead
	// format (leftX, rightX, topY, bottomY)
	void setBounds(my_vec4 coords) {
		bounds.left		= coords.x;
		bounds.right	= coords.y;
		bounds.top		= coords.z;
		bounds.bottom	= coords.w;

		this->height = bounds.right - bounds.left;
		this->height = bounds.bottom - bounds.top;
	}

	void setBounds(AABB inBounds) {
		this->bounds.left	= inBounds.left;
		this->bounds.right	= inBounds.right;
		this->bounds.top	= inBounds.top;
		this->bounds.bottom = inBounds.bottom;

		this->height = bounds.right - bounds.left;
		this->height = bounds.bottom - bounds.top;
	}

	void setBounds(my_vec2 topLeft, float inHeight, float inWidth) {
		bounds.set(topLeft, inHeight, inWidth);
		this->height;
		this->width;
	}

	void draw() {
		glUseProgram(this->shaderProgramID);
		setUniform4f(this->shaderProgramID, "color", my_vec4(color.x, color.y, color.z, color.w));

		glBindVertexArray(VAO_ID);

		float vertices[] = {
			bounds.left,	bounds.top,
			bounds.right,	bounds.top,
			bounds.left,	bounds.bottom,

			bounds.right,	bounds.top,
			bounds.left,	bounds.bottom,
			bounds.right,	bounds.bottom
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	boolean click(my_vec2 clickCoords) {
		if (clickCoords.x < bounds.AX) return false;
		if (clickCoords.x > bounds.BX) return false;
		if (clickCoords.y > bounds.AY) return false;
		if (clickCoords.y < bounds.BY) return false;

		return true;
	}
};
