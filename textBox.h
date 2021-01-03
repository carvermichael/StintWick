#pragma once

#pragma warning (push, 0)
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma warning (pop)

#include "constants.h"
#include "math.h"
#include <map>
#include <iostream>

struct TextCharacter {
	unsigned int textureID;
	my_ivec2	 size;
	my_ivec2	 bearing;
	unsigned int advance;
};

struct Font {
	std::map<char, TextCharacter> textCharacters;
	unsigned int VAO_ID, VBO_ID;
	unsigned int shaderProgramID;

	Font();

	void init(const char *fontFileName, unsigned int shaderProgramID);
};

struct Textbox {
	std::string lines[LIMIT_LINES];
	unsigned int numLinesUsed = 0;
	unsigned int maxLinesToShow = 4;
    bool flip = false;

	float x = 0.0f;
	float y = 0.0f;

	void drawTextBox(Font *font);

	void addTextToBox(std::string newText);

	void clearTextBox();

	void addTextToBoxAtLine(std::string newText, int lineNum);
};
