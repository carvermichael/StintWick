#if !defined(CONSOLE)

#include "textBox.h"
#include "UIRect.h"

struct Console {
	Font *font;
	
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

	float currentScreenWidth;
	float currentScreenHeight;

	float height = 100.0f;
	float width;

	float location;
	float destination;
	float speed = 10.0f;

	void flipOut();

	void refresh(float screenWidth, float screenHeight);

	void setup(unsigned int shaderProgramId, float screenWidth, float screenHeight, Font *font);

	void addInput(char input);

	void removeCharacter();

	void submit();

	void draw();

	void update(float deltaTime);
};

#define CONSOLE 
#endif
