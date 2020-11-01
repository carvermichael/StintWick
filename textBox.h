#if !defined(TEXTBOX)
#pragma warning (push, 0)
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma warning (pop)

#include "math.h"
#include "global_manip.h"
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

	Font(const char *fontFileName, unsigned int shaderProgramID) {
		// TEXT RENDERING
		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		}

		FT_Face face;
		if (FT_New_Face(ft, fontFileName, 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		}

		// 0 for width means dynamically adjust based on height
		FT_Set_Pixel_Sizes(face, 0, 48);

		// generate a texture for each ASCII character
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

		for (unsigned char c = 0; c < 128; c++) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			}

			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			TextCharacter textCharacter = {
				texture,
				my_ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				my_ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				(unsigned int) face->glyph->advance.x
			};

			textCharacters.insert(std::pair<char, TextCharacter>(c, textCharacter));
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		// TEXT SHADER PROGRAM
		this->shaderProgramID = shaderProgramID;

		// reserving data for text on gpu
		glGenVertexArrays(1, &this->VAO_ID);
		glGenBuffers(1, &this->VBO_ID);
		glBindVertexArray(this->VAO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO_ID);

		/*
			Note the NULL:
			So far, in other calls like this, we give a pointer to the data at the same time we describe the data.
			However, this time we're going to create each text character's vertices on the fly (as they each have their
			own spacial needs. So, this first Data call just describes the data, and the subData call later sends along
			the vertices. -- carver (7-27-20)
		*/
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

inline void drawText(Font *font, std::string text, float x, float y, float scale, my_vec3 color) {

	setUniform3f(font->shaderProgramID, "textColor", color);

	// TODO: DON'T FORGET TO WORK OUT THE TEXTURE INFO HERE TOO -- can't just use texture 0 for all fonts
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(font->VAO_ID);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		TextCharacter ch = font->textCharacters[*c];

		float xPos = x + ch.bearing.x * scale;
		float yPos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		float vertices[] = {
			xPos,		yPos + h, 0.0f, 0.0f,
			xPos,		yPos,	  0.0f, 1.0f,
			xPos + w,	yPos,     1.0f, 1.0f,

			xPos,		yPos + h, 0.0f, 0.0f,
			xPos + w,	yPos,	  1.0f, 1.0f,
			xPos + w,	yPos + h, 1.0f, 0.0f,
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, font->VBO_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// this is a lot of draw calls per line of text
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.advance >> 6) * scale; // bit shift changes unit to pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#define LIMIT_LINES 2000

struct Textbox {
	std::string lines[LIMIT_LINES];
	unsigned int numLinesUsed = 0;
	unsigned int maxLinesToShow = 4;
    bool flip = false;

	float x = 0.0f;
	float y = 0.0f;

	void drawTextBox(Font *font) {
		int startingLineIndex;
		int numLinesToShow;
		if (numLinesUsed < maxLinesToShow) {
			startingLineIndex = 0;
			numLinesToShow = numLinesUsed;
		}
		else {
			startingLineIndex = numLinesUsed - maxLinesToShow;
			numLinesToShow = maxLinesToShow;
		}

		if (!flip) {
			for (int i = startingLineIndex; i < numLinesToShow + startingLineIndex; i++) {
				drawText(font, lines[i], x, y, 0.4f, my_vec3(1.0f, 0.5f, 0.89f));

				y += 20.0f;
			}
		}
		else {
			for (int i = numLinesToShow + startingLineIndex - 1; i >= startingLineIndex; i--) {
				drawText(font, lines[i], x, y, 0.4f, my_vec3(1.0f, 0.5f, 0.89f));

				y += 20.0f;
			}
		}
	}

	// TODO: This needs wraparound.
	void addTextToBox(std::string newText) {
		lines[numLinesUsed] = newText;

		numLinesUsed++;
	}

	void clearTextBox() {
		numLinesUsed = 0;
	}


	void addTextToBoxAtLine(std::string newText, int lineNum) {
		lines[lineNum] = newText;
		numLinesUsed = 1;
	}
};



#define TEXTBOX
#endif
