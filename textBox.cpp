#include "textBox.h"
#include "global_manip.h"

// FONT
Font::Font() {};

void Font::init(const char *fontFileName, unsigned int shaderProgramID) {
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
			(unsigned int)face->glyph->advance.x
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

// TEXTBOX
void Textbox::drawTextBox(Font *font) {
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

void Textbox::addTextToBox(std::string newText) {
	lines[numLinesUsed] = newText;

	numLinesUsed++;
}

void Textbox::clearTextBox() {
	numLinesUsed = 0;
}


void Textbox::addTextToBoxAtLine(std::string newText, int lineNum) {
	lines[lineNum] = newText;
	numLinesUsed = 1;
}
