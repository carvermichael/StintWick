#include "global_manip.h"
#include "console.h"

void Console::flipOut() {
	if (isOut) {
		destination = currentScreenHeight;
	}
	else {
		destination = currentScreenHeight - height;
	}

	isOut = !isOut;
}

void Console::refresh(float screenWidth, float screenHeight) {
	timeToFlip = 0.5f;
	markerSize = 1.5f;
	markerStartLocation = 4.0f;
	markerLocation = markerStartLocation;

	currentScreenWidth = screenWidth;
	currentScreenHeight = screenHeight;

	height = 100.0f;
	width = currentScreenWidth;

	location = currentScreenHeight;
	destination = currentScreenHeight;
	speed = 10.0f;

	isOut = false;
}

void Console::setup(unsigned int shaderProgramId, float screenWidth, float screenHeight, Font *font) {
	this->font = font;

	boundingRect.setup(shaderProgramId);
	boundingRect.color = my_vec4(0.2f, 0.2f, 0.2f, 1.0f);

	inputRect.setup(shaderProgramId);
	inputRect.color = my_vec4(0.1f, 0.1f, 0.1f, 1.0f);

	editMarkerRect.setup(shaderProgramId);
	editMarkerRect.color = my_vec4(0.1f, 0.1f, 0.1f, 1.0f);

	historyTextbox.maxLinesToShow = 10;
	historyTextbox.flip = true;

	refresh(screenWidth, screenHeight);
}

void Console::addInput(char input) {
	inputString.push_back(input);
}

void Console::removeCharacter() {
	if (inputString.empty()) return;

	inputString.pop_back();
}

void Console::submit() {
	if (inputString.empty()) return;

	historyTextbox.addTextToBox(inputString);

	processConsoleCommand(inputString);

	inputString.clear();
}

void Console::draw() {

	boundingRect.draw();

	inputRect.draw();

	//timeSinceEditMarkerFlip += deltaTime;
	//if (timeSinceEditMarkerFlip > timeToFlip) {
	//	editMarkerOn = !editMarkerOn;
	//	timeSinceEditMarkerFlip = 0; // should this be the remainder??
	//}

	//if (editMarkerOn) {
	//	editMarkerRect.setBounds(my_vec4(markerLocation, markerLocation + markerSize, historyTextbox.y + 20.0f, historyTextbox.y));
	//	editMarkerRect.draw();
	//}

	drawText(this->font, inputString, 0.0f, location + 5.0f, 0.4f, my_vec3(1.0f, 1.0f, 1.0f));

	historyTextbox.drawTextBox(this->font);
}

void Console::update(float deltaTime) {
	float dist = destination - location;
	if (glm::abs(dist) < 1) location = destination;

	float distToMove = dist / 4.0f * deltaTime * speed;

	location += distToMove;

	historyTextbox.y = location + 27.0f;

	boundingRect.setBounds(AABB(0.0f, (float)currentScreenWidth, location, (float)currentScreenHeight));
	inputRect.setBounds(AABB(0.0f, (float)currentScreenWidth, location + 25.0f, location));
}
