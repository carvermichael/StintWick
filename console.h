#if !defined(CONSOLE)
#include "textBox.h"
#include "model.h"

struct UI_Rect {

	// TODO: figure out why the linker threw errors when you tried to make some of these static
	bool initialized;

	unsigned int VAO_ID;
	unsigned int VBO_ID;
	unsigned int shaderProgramID; // do we even need programIDs on structs if those IDs will be in global state?

	float leftX, rightX, topY, bottomY;

	glm::vec3 color;
	float alpha;

	// should only need to call this once
	void setup() {
		if (initialized) return;

		glGenVertexArrays(1, &VAO_ID);
		glGenBuffers(1, &VBO_ID);
		glBindVertexArray(VAO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		color = glm::vec3(0.2f, 0.2f, 0.2f);
		alpha = 0.7f;

		shaderProgramID = UIShaderProgramID;
		// TODO: this probably can happen elsewhere and only once 
		//		(this shader will have the same projection matrix every time)
		glUseProgram(UIShaderProgramID);

		glm::mat4 projection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
		unsigned int projectionLoc = glGetUniformLocation(UIShaderProgramID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

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

		glUseProgram(UIShaderProgramID);
		unsigned int colorLoc = glGetUniformLocation(UIShaderProgramID, "color");
		glUniform4f(colorLoc, color.x, color.y, color.z, alpha);

		glm::mat4 projection = glm::ortho(0.0f, (float)currentScreenWidth, 0.0f, (float)currentScreenHeight);
		unsigned int projectionLoc = glGetUniformLocation(UIShaderProgramID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
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
		glBindBuffer(GL_ARRAY_BUFFER, 0); // not sure why this is here

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

struct Console {
	Textbox historyTextbox;
	UI_Rect boundingRect;
	UI_Rect inputRect;

	bool consoleOut = false;

	// Edit Marker Stuffs
	UI_Rect editMarkerRect;
	bool editMarkerOn;
	float timeSinceEditMarkerFlip;
	float timeToFlip = 0.5f;
	float markerSize = 1.5f;
	float markerStartLocation = 4.0f;
	float markerLocation = markerStartLocation;

	

	// TODO: will need to update a lot of this when the screen size changes
	float height = 200.0f;
	float width = (float)currentScreenWidth;

	float location = (float)currentScreenHeight;
	float destination = (float)currentScreenHeight;
	float speed = 10.0f;

	void flipOut() {
		if (consoleOut) {
			destination = (float)currentScreenHeight;
		} else {
			destination = (float)(currentScreenHeight - height);
		}

		consoleOut = !consoleOut;
	}

	void setup() {
		boundingRect.setup();

		inputRect.setup();
		inputRect.color = glm::vec3(0.1f, 0.1f, 0.1f);
		inputRect.alpha = 1.0f;
		
		editMarkerRect.setup();
		inputRect.color = glm::vec3(0.1f, 0.1f, 0.1f);
		inputRect.alpha = 1.0f;
	}

	void draw(float deltaTime) {
		float dist = destination - location;
		if (glm::abs(dist) < 1) location = destination;

		float distToMove = dist / 4.0f * deltaTime * speed;
		
		location += distToMove;
		
		historyTextbox.y = location;

		boundingRect.setCoords(glm::vec4(0.0f, currentScreenWidth, historyTextbox.y, currentScreenHeight));
		boundingRect.draw();

		inputRect.setCoords(glm::vec4(0.0f, currentScreenWidth, historyTextbox.y + 20.0f, historyTextbox.y));
		inputRect.draw();

		timeSinceEditMarkerFlip += deltaTime;
		if (timeSinceEditMarkerFlip > timeToFlip) {
			editMarkerOn = !editMarkerOn;
			timeSinceEditMarkerFlip = 0; // this should probably be the remainder
		}

		if (editMarkerOn) {
			editMarkerRect.setCoords(glm::vec4(markerLocation, markerLocation + markerSize, historyTextbox.y + 20.0f, historyTextbox.y));
			editMarkerRect.draw();
		}

		drawTextBox(&historyTextbox);
	}
};

#define CONSOLE 
#endif