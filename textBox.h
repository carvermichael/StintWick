#if !defined(TEXTBOX)

struct TextCharacter {
	unsigned int textureID;
	glm::ivec2	 size;
	glm::ivec2	 bearing;
	unsigned int advance;
};

struct Font {
	std::map<char, TextCharacter> textCharacters;
	unsigned int VAO_ID, VBO_ID;
	unsigned int shaderProgramID;
};

Font ariel;

void initializeFont(const char *fontFileName, Font *font) {
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
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};

		font->textCharacters.insert(std::pair<char, TextCharacter>(c, textCharacter));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// TEXT SHADER PROGRAM
	font->shaderProgramID = createShaderProgram("textVertexShader.vert", "textFragmentShader.frag");

	// reserving data for text on gpu
	glGenVertexArrays(1, &font->VAO_ID);
	glGenBuffers(1, &font->VBO_ID);
	glBindVertexArray(font->VAO_ID);
	glBindBuffer(GL_ARRAY_BUFFER, font->VBO_ID);

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

void drawText(Font *font, std::string text, float x, float y, float scale, glm::vec3 color) {

	glUseProgram(font->shaderProgramID);
	unsigned int textColorLoc = glGetUniformLocation(font->shaderProgramID, "textColor");
	glUniform3f(textColorLoc, color.x, color.y, color.z);

	glm::mat4 textProjection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
	unsigned int projectionLoc = glGetUniformLocation(font->shaderProgramID, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(textProjection));

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

		float vertices[6][4] = {
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

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.advance >> 6) * scale; // bit shift changes unit to pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#define LIMIT_LINES 6

struct Textbox {
	std::string lines[LIMIT_LINES];
	unsigned int startingLineIndex = 0;

	float x = 0.0f;
	float y = 0.0f;

	Font *font;
};

Textbox eventTextBox = {};

void drawTextBox(Textbox *textbox) {
	unsigned int numLinesRendered = 0;
	unsigned int currentLineIndex = eventTextBox.startingLineIndex;
	float x = 0.0f, y = 0.0f;

	while (numLinesRendered < LIMIT_LINES) {
		drawText(textbox->font, eventTextBox.lines[currentLineIndex], x, y, 0.4f, glm::vec3(1.0f, 0.5f, 0.89f));

		currentLineIndex++;
		if (currentLineIndex >= LIMIT_LINES) {
			currentLineIndex = 0;
		}

		y += 20.0f;

		numLinesRendered++;
	}
}

void addTextToBox(std::string newText, Textbox *textbox) {
	textbox->lines[textbox->startingLineIndex] = newText;

	textbox->startingLineIndex++;
	if (textbox->startingLineIndex >= LIMIT_LINES) {
		textbox->startingLineIndex = 0;
	}
}

#define TEXTBOX
#endif