#if !defined(CONSOLE)
#include "textBox.h"
#include <string>

struct Console {
	unsigned int shaderProgramID;

	std::string lines[LIMIT_LINES];
	unsigned int startingLineIndex = 0;
	
	unsigned int location; // y-coord on viewport (from top)
	unsigned int destination;
	unsigned int speed;

	void initialize(char *fontFileName) {

	}
};



#define CONSOLE
#endif