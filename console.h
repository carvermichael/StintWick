#if !defined(CONSOLE)

#include "textBox.h"

struct UI_Rect {

	// TODO: figure out why the linker threw errors when you tried to make some of these static
	bool initialized;

	unsigned int VAO_ID;
	unsigned int VBO_ID;
	unsigned int shaderProgramID;

	float leftX, rightX, topY, bottomY;

	glm::vec3 color;
	float alpha;

	// should only need to call this once
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

		color = glm::vec3(0.2f, 0.2f, 0.2f);
		alpha = 0.7f;

		this->shaderProgramID = shaderProgramId;

		initialized = true;
	}

	// format (leftX, rightX, topY, bottomY)
	void setCoords(glm::vec4 coords) {
		leftX	= coords.x;
		rightX	= coords.y;
		topY	= coords.z;
		bottomY = coords.w;
	}

	void draw() {
		glUseProgram(this->shaderProgramID);
		setUniform4f(this->shaderProgramID, "color", glm::vec4(color.x, color.y, color.z, alpha));

		glBindVertexArray(VAO_ID);

		float vertices[] = {
			leftX,	topY,
			rightX, topY,
			leftX,	bottomY,

			rightX, topY,
			leftX,	bottomY,
			rightX, bottomY
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

struct Console {
	Textbox historyTextbox;
	UI_Rect boundingRect;
	UI_Rect inputRect;
	std::string inputString;

	bool isOut = false;

	// Edit Marker Stuffs
	UI_Rect editMarkerRect;
	bool editMarkerOn;
	float timeSinceEditMarkerFlip;
	float timeToFlip = 0.5f;
	float markerSize = 1.5f;
	float markerStartLocation = 4.0f;
	float markerLocation = markerStartLocation;

	float height = 100.0f;
	float width = (float)currentScreenWidth;

	float location = (float)currentScreenHeight;
	float destination = (float)currentScreenHeight;
	float speed = 10.0f;

	void flipOut() {
		if (isOut) {
			destination = (float)currentScreenHeight;
		} else {
			destination = (float)(currentScreenHeight - height);
		}

		isOut = !isOut;
	}

	void refresh() {
		timeToFlip = 0.5f;
		markerSize = 1.5f;
		markerStartLocation = 4.0f;
		markerLocation = markerStartLocation;

		height = 100.0f;
		width = (float)currentScreenWidth;

		location = (float)currentScreenHeight;
		destination = (float)currentScreenHeight;
		speed = 10.0f;

		isOut = false;
	}

	void setup(unsigned int shaderProgramId) {
		boundingRect.setup(shaderProgramId);

		inputRect.setup(shaderProgramId);
		inputRect.color = glm::vec3(0.1f, 0.1f, 0.1f);
		inputRect.alpha = 0.7f;
		
		editMarkerRect.setup(shaderProgramId);
		editMarkerRect.color = glm::vec3(0.1f, 0.1f, 0.1f);
		editMarkerRect.alpha = 1.0f;

		historyTextbox.maxLinesToShow = 10;
		historyTextbox.flip = true;
	}

	void addInput(char input) {
		inputString.push_back(input);
	}

	void removeCharacter() {
		if (inputString.empty()) return;
		
		inputString.pop_back();
	}

	void submit() {
		if (inputString.empty()) return;

		addTextToBox(inputString, &historyTextbox);
		
		processConsoleCommand(inputString);

		inputString.clear();
	}

	void draw(float deltaTime, Font *font) {
		float dist = destination - location;
		if (glm::abs(dist) < 1) location = destination;

		float distToMove = dist / 4.0f * deltaTime * speed;
		
		location += distToMove;
		
		historyTextbox.y = location + 27.0f;

		boundingRect.setCoords(glm::vec4(0.0f, currentScreenWidth, location, currentScreenHeight));
		boundingRect.draw();

		inputRect.setCoords(glm::vec4(0.0f, currentScreenWidth, location + 25.0f, location));
		inputRect.draw();

		//timeSinceEditMarkerFlip += deltaTime;
		//if (timeSinceEditMarkerFlip > timeToFlip) {
		//	editMarkerOn = !editMarkerOn;
		//	timeSinceEditMarkerFlip = 0; // should this be the remainder??
		//}

		//if (editMarkerOn) {
		//	editMarkerRect.setCoords(glm::vec4(markerLocation, markerLocation + markerSize, historyTextbox.y + 20.0f, historyTextbox.y));
		//	editMarkerRect.draw();
		//}

		drawText(font, inputString, 0.0f, location + 5.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));

		drawTextBox(&historyTextbox, font);
	}
};

#define CONSOLE 
#endif
